#ifdef DrawText
#undef DrawText
#endif
#include "ContentManager.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/array.hpp"
#include "ContentProcessorRegistration.hpp"
#include "IContentProcessor_1.hpp"
#include "EngineBinaryReadContext.hpp"
#include "system/io/file-stream.hpp"
#include "system/io/file.hpp"
#include "system/io/path.hpp"
#include "ContentManager.hpp"
#include "TextContentProcessor.hpp"
#include "RawByteContentProcessor.hpp"
#include "runtime/native_type.hpp"
#include "IContentProcessor.hpp"
#include "system/io/stream.hpp"
#include "TextContent.hpp"
#include "RawByteContent.hpp"
#include "system/string_comparer.hpp"
#include "system/collections/generic/ienumerable.hpp"
#include "system/io/file.hpp"
#include "system/io/file-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "system/collections/generic/ienumerable.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"
#include "system/io/path.hpp"
#include "system/io/stream.hpp"
#include "system/string_comparer.hpp"

const std::string& ContentManager::get_RootDirectory()
{
return this->RootDirectoryPath;
}

ContentManager::ContentManager(std::string rootDirectory) : RootDirectoryPath(), ProcessorRegistrationsById(), DefaultProcessorsByTypeAndExtension()
{
    if (String::IsNullOrWhiteSpace(rootDirectory))
    {
throw ([&]() {
auto __ctor_arg_0000003B = "Root directory must be provided.";
auto __ctor_arg_0000003C = "rootDirectory";
return new ArgumentException(__ctor_arg_0000003B, __ctor_arg_0000003C);
})();
    }
this->RootDirectoryPath = ContentManager::HasVirtualRootPrefix(rootDirectory) ? rootDirectory : Path::GetFullPath(rootDirectory);
this->ProcessorRegistrationsById = new Dictionary<std::string, ::ContentProcessorRegistration*>(StringComparer::get_OrdinalIgnoreCase());
this->DefaultProcessorsByTypeAndExtension = new Dictionary<Type*, Dictionary<std::string, ::ContentProcessorRegistration*>*>();
this->RegisterBuiltInProcessors();
}

bool ContentManager::IsProcessorRegistered(std::string processorId)
{
    if (String::IsNullOrWhiteSpace(processorId))
    {
throw ([&]() {
auto __ctor_arg_0000003D = "Processor id must be provided.";
auto __ctor_arg_0000003E = "processorId";
return new ArgumentException(__ctor_arg_0000003D, __ctor_arg_0000003E);
})();
    }
return this->ProcessorRegistrationsById->ContainsKey(processorId);}

template <typename T>
T ContentManager::Load(std::string assetPath, std::string processorId)
{
const std::string fullPath = this->ResolveContentPath(assetPath);
::IContentProcessor_1<T> *processor = ResolveProcessor<T>(fullPath, processorId);
return this->LoadProcessedContent<T>(fullPath, processor);}

template <typename T>
T ContentManager::Load(std::string assetPath, ::IContentProcessor_1<T>* processor)
{
    if (processor == nullptr)
    {
throw new ArgumentNullException("processor");
    }
const std::string fullPath = this->ResolveContentPath(assetPath);
return this->LoadProcessedContent<T>(fullPath, processor);}

void ContentManager::RegisterProcessor(::ContentProcessorRegistration* registration)
{
    if (registration == nullptr)
    {
throw new ArgumentNullException("registration");
    }
    if (this->ProcessorRegistrationsById->ContainsKey(registration->get_ProcessorId()))
    {
throw new InvalidOperationException(std::string("Content processor '") + registration->get_ProcessorId() + std::string("' is already registered."));
    }
Array<std::string> *extensions = registration->get_Extensions();
    if (extensions->get_Length() > 0)
    {
Dictionary<std::string, ::ContentProcessorRegistration*> *registrationsByExtension = this->GetOrCreateTypeRegistrationMap(registration->get_OutputType());
for (int32_t extensionIndex = 0; extensionIndex < extensions->get_Length(); extensionIndex++) {
const std::string extension = this->NormalizeExtension((*extensions)[extensionIndex]);
    if (registrationsByExtension->ContainsKey(extension))
    {
throw new InvalidOperationException(std::string("A default content processor is already registered for type '") + registration->get_OutputType()->get_Name() + std::string("' and extension '") + extension + std::string("'."));
    }
registrationsByExtension->Add(extension, registration);
}
    }
this->ProcessorRegistrationsById->Add(registration->get_ProcessorId(), registration);
}

template <typename T>
void ContentManager::RegisterProcessor(std::string processorId, ::IContentProcessor_1<T>* processor, Array<std::string>* extensions)
{
    if (processor == nullptr)
    {
throw new ArgumentNullException("processor");
    }
this->RegisterProcessor(new ::ContentProcessorRegistration(processorId, processor, extensions));
}

std::string ContentManager::CombineVirtualRootedPath(std::string rootPath, std::string relativePath)
{
    if (String::IsNullOrWhiteSpace(rootPath))
    {
throw ([&]() {
auto __ctor_arg_0000003F = "Root path must be provided.";
auto __ctor_arg_00000040 = "rootPath";
return new ArgumentException(__ctor_arg_0000003F, __ctor_arg_00000040);
})();
    }
    if (String::IsNullOrWhiteSpace(relativePath))
    {
throw ([&]() {
auto __ctor_arg_00000041 = "Relative path must be provided.";
auto __ctor_arg_00000042 = "relativePath";
return new ArgumentException(__ctor_arg_00000041, __ctor_arg_00000042);
})();
    }
return String::Concat(ContentManager::EnsureTrailingDirectorySeparator(rootPath), ContentManager::TrimLeadingDirectorySeparators(relativePath));}

std::string ContentManager::EnsureTrailingDirectorySeparator(std::string path)
{
    if (String::EndsWith(path, Path::DirectorySeparatorChar) || String::EndsWith(path, Path::AltDirectorySeparatorChar))
    {
return path;    }
return String::Concat(path, std::string(1, Path::DirectorySeparatorChar));}

Dictionary<std::string, ::ContentProcessorRegistration*>* ContentManager::GetOrCreateTypeRegistrationMap(Type* outputType)
{
    if (outputType == nullptr)
    {
throw new ArgumentNullException("outputType");
    }
Dictionary<std::string, ::ContentProcessorRegistration*>* registrationsByExtension;
    if (this->DefaultProcessorsByTypeAndExtension->TryGetValue(outputType, registrationsByExtension))
    {
return registrationsByExtension;    }
registrationsByExtension = new Dictionary<std::string, ::ContentProcessorRegistration*>(StringComparer::get_OrdinalIgnoreCase());
this->DefaultProcessorsByTypeAndExtension->Add(outputType, registrationsByExtension);
return registrationsByExtension;}

bool ContentManager::HasVirtualRootPrefix(std::string path)
{
    if (String::IsNullOrWhiteSpace(path))
    {
return false;    }
int32_t colonIndex = -1;
for (int32_t index = 0; index < static_cast<int32_t>(path.size()); index++) {
    if (path[index] == ':')
    {
colonIndex = index;
break;
    }
}
    if (colonIndex <= 0)
    {
return false;    }
else {
    if (colonIndex == 1)
    {
return false;    }
else {
    if (colonIndex >= static_cast<int32_t>(path.size()) - 1)
    {
return false;    }
}
}
const char nextCharacter = path[colonIndex + 1];
return nextCharacter == Path::DirectorySeparatorChar || nextCharacter == Path::AltDirectorySeparatorChar;}

template <typename T>
T ContentManager::LoadProcessedContent(std::string fullPath, ::IContentProcessor_1<T>* processor)
{
    if (String::IsNullOrWhiteSpace(fullPath))
    {
throw ([&]() {
auto __ctor_arg_00000043 = "Content path must be provided.";
auto __ctor_arg_00000044 = "fullPath";
return new ArgumentException(__ctor_arg_00000043, __ctor_arg_00000044);
})();
    }
    if (processor == nullptr)
    {
throw new ArgumentNullException("processor");
    }
const std::string previousAssetPath = EngineBinaryReadContext::get_CurrentAssetPath();
const std::string previousReadStage = EngineBinaryReadContext::get_CurrentReadStage();
EngineBinaryReadContext::set_CurrentAssetPath(fullPath);
EngineBinaryReadContext::set_CurrentReadStage("ContentManager:OpenRead");
{
auto __finallyGuard_00000045 = he_cpp_make_scope_exit([&]() {
EngineBinaryReadContext::set_CurrentAssetPath(previousAssetPath);
EngineBinaryReadContext::set_CurrentReadStage(previousReadStage);
});
{
::FileStream *stream = File::OpenRead(fullPath);
auto __usingDisposeGuard_00000046 = he_cpp_make_scope_exit([&]() {
if (stream != nullptr) {
stream->Dispose();
delete stream;
}
});
EngineBinaryReadContext::set_CurrentReadStage("ContentManager:ProcessorRead");
return processor->Read(stream);}
}
}

std::string ContentManager::NormalizeExtension(std::string extension)
{
    if (String::IsNullOrWhiteSpace(extension))
    {
throw ([&]() {
auto __ctor_arg_00000047 = "Extension must be provided.";
auto __ctor_arg_00000048 = "extension";
return new ArgumentException(__ctor_arg_00000047, __ctor_arg_00000048);
})();
    }
    if (String::Equals(extension, WildcardExtension, StringComparison::Ordinal))
    {
return extension;    }
    if (!String::StartsWith(extension, "."))
    {
extension = String::Concat(".", extension);
    }
return String::ToLowerInvariant(extension);}

void ContentManager::RegisterBuiltInProcessors()
{
this->RegisterProcessor<TextContent*>(TextContentProcessorId, new ::TextContentProcessor(), new Array<std::string>({ WildcardExtension }));
this->RegisterProcessor<RawByteContent*>(RawByteContentProcessorId, new ::RawByteContentProcessor(), new Array<std::string>({ WildcardExtension }));
}

std::string ContentManager::ResolveContentPath(std::string assetPath)
{
    if (String::IsNullOrWhiteSpace(assetPath))
    {
throw ([&]() {
auto __ctor_arg_00000049 = "Asset path must be provided.";
auto __ctor_arg_0000004A = "assetPath";
return new ArgumentException(__ctor_arg_00000049, __ctor_arg_0000004A);
})();
    }
    if (ContentManager::HasVirtualRootPrefix(assetPath))
    {
return assetPath;    }
    if (Path::IsPathRooted(assetPath))
    {
return Path::GetFullPath(assetPath);    }
    if (ContentManager::HasVirtualRootPrefix(this->RootDirectoryPath))
    {
return ContentManager::CombineVirtualRootedPath(this->RootDirectoryPath, assetPath);    }
return Path::GetFullPath(Path::Combine(this->RootDirectoryPath, assetPath));}

::ContentProcessorRegistration* ContentManager::ResolveDefaultProcessorRegistration(Type* requestedType, std::string fullPath)
{
    if (requestedType == nullptr)
    {
throw new ArgumentNullException("requestedType");
    }
    if (String::IsNullOrWhiteSpace(fullPath))
    {
throw ([&]() {
auto __ctor_arg_0000004B = "Content path must be provided.";
auto __ctor_arg_0000004C = "fullPath";
return new ArgumentException(__ctor_arg_0000004B, __ctor_arg_0000004C);
})();
    }
Dictionary<std::string, ::ContentProcessorRegistration*>* registrationsByExtension;
    if (!this->DefaultProcessorsByTypeAndExtension->TryGetValue(requestedType, registrationsByExtension))
    {
throw new InvalidOperationException(std::string("No content processors are registered for type '") + requestedType->get_Name() + std::string("'."));
    }
const std::string fileName = Path::GetFileName(fullPath);
    if (String::IsNullOrWhiteSpace(fileName))
    {
throw new InvalidOperationException(std::string("Unable to resolve a content processor for '") + requestedType->get_Name() + std::string("' because '") + fullPath + std::string("' does not contain a file name."));
    }
std::string extension;
    if (!this->TryResolveRegisteredExtension__out2(fileName, registrationsByExtension, extension))
    {
throw new InvalidOperationException(std::string("No content processor is registered for type '") + requestedType->get_Name() + std::string("' and file '") + fileName + std::string("'."));
    }
::ContentProcessorRegistration* registration;
    if (registrationsByExtension->TryGetValue(extension, registration))
    {
return registration;    }
throw new InvalidOperationException(std::string("No content processor is registered for type '") + requestedType->get_Name() + std::string("' and extension '") + extension + std::string("'."));
}

::ContentProcessorRegistration* ContentManager::ResolveExplicitProcessorRegistration(Type* requestedType, std::string processorId)
{
    if (requestedType == nullptr)
    {
throw new ArgumentNullException("requestedType");
    }
    if (String::IsNullOrWhiteSpace(processorId))
    {
throw ([&]() {
auto __ctor_arg_0000004D = "Processor id must be provided.";
auto __ctor_arg_0000004E = "processorId";
return new ArgumentException(__ctor_arg_0000004D, __ctor_arg_0000004E);
})();
    }
::ContentProcessorRegistration* registration;
    if (!this->ProcessorRegistrationsById->TryGetValue(processorId, registration))
    {
throw new InvalidOperationException(std::string("Content processor '") + processorId + std::string("' is not registered."));
    }
    if (registration->get_OutputType() != requestedType)
    {
throw new InvalidOperationException(std::string("Content processor '") + processorId + std::string("' produces '") + registration->get_OutputType()->get_Name() + std::string("', not '") + requestedType->get_Name() + std::string("'."));
    }
return registration;}

template <typename T>
::IContentProcessor_1<T>* ContentManager::ResolveProcessor(std::string fullPath, std::string processorId)
{
::ContentProcessorRegistration *registration = String::IsNullOrWhiteSpace(processorId) ? this->ResolveDefaultProcessorRegistration(he_cpp_type_of<T>("T"), fullPath) : this->ResolveExplicitProcessorRegistration(he_cpp_type_of<T>("T"), processorId);
    IContentProcessor_1<T>* typedProcessor = he_cpp_try_cast<IContentProcessor_1<T>>(registration->get_Processor());
    if (typedProcessor != nullptr)
    {
return typedProcessor;    }
throw new InvalidOperationException(std::string("Registered processor '") + registration->get_ProcessorId() + std::string("' does not implement the expected processor interface for type '") + he_cpp_type_of<T>("T")->get_Name() + std::string("'."));
}

std::string ContentManager::TrimLeadingDirectorySeparators(std::string path)
{
    if (String::IsNullOrWhiteSpace(path))
    {
return String::Empty;    }
int32_t startIndex = 0;
while (startIndex < static_cast<int32_t>(path.size()) && (path[startIndex] == Path::DirectorySeparatorChar || path[startIndex] == Path::AltDirectorySeparatorChar)) {
startIndex++;
}
    if (startIndex == 0)
    {
return path;    }
    if (startIndex >= static_cast<int32_t>(path.size()))
    {
return String::Empty;    }
return String::Substring(path, startIndex);}

bool ContentManager::TryResolveRegisteredExtension__out2(std::string fileName, Dictionary<std::string, ::ContentProcessorRegistration*>* registrationsByExtension, std::string& matchedExtension)
{
    if (String::IsNullOrWhiteSpace(fileName))
    {
throw ([&]() {
auto __ctor_arg_0000004F = "File name must be provided.";
auto __ctor_arg_00000050 = "fileName";
return new ArgumentException(__ctor_arg_0000004F, __ctor_arg_00000050);
})();
    }
    if (registrationsByExtension == nullptr)
    {
throw new ArgumentNullException("registrationsByExtension");
    }
const std::string normalizedFileName = String::ToLowerInvariant(fileName);
matchedExtension = String::Empty;
for (const auto& extension : registrationsByExtension->Keys()) {
    if (String::Equals(extension, WildcardExtension, StringComparison::Ordinal))
    {
    if (String::IsNullOrWhiteSpace(matchedExtension))
    {
matchedExtension = extension;
    }
continue;
    }
    if (!String::EndsWith(normalizedFileName, extension, StringComparison::OrdinalIgnoreCase))
    {
continue;
    }
    if (static_cast<int32_t>(matchedExtension.size()) >= static_cast<int32_t>(extension.size()))
    {
continue;
    }
matchedExtension = extension;
}
return !String::IsNullOrWhiteSpace(matchedExtension);}

