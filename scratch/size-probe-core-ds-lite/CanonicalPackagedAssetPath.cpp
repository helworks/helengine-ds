#ifdef DrawText
#undef DrawText
#endif
#include "CanonicalPackagedAssetPath.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/text/string-builder.hpp"
#include "CanonicalPackagedAssetPath.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_string.hpp"
#include "system/text/string-builder.hpp"

std::string CanonicalPackagedAssetPath::Normalize(std::string path)
{
    if (String::IsNullOrWhiteSpace(path))
    {
throw ([&]() {
auto __ctor_arg_00000037 = "Packaged asset path must be provided.";
auto __ctor_arg_00000038 = "path";
return new ArgumentException(__ctor_arg_00000037, __ctor_arg_00000038);
})();
    }
const std::string normalizedPath = CanonicalPackagedAssetPath::NormalizeSlashAndCase(String::Trim(path));
    if (CanonicalPackagedAssetPath::IsRootedPath(normalizedPath))
    {
throw new InvalidOperationException(std::string("Packaged asset path '") + path + std::string("' must be relative."));
    }
const std::string canonicalPath = CanonicalPackagedAssetPath::CollapseAndValidateSegments(normalizedPath, path);
    if (static_cast<int32_t>(canonicalPath.size()) == 0)
    {
throw new InvalidOperationException(std::string("Packaged asset path '") + path + std::string("' must not be empty."));
    }
return canonicalPath;}

std::string CanonicalPackagedAssetPath::ValidateCanonical(std::string path)
{
const std::string normalizedPath = CanonicalPackagedAssetPath::Normalize(path);
    if (!String::Equals(normalizedPath, path, StringComparison::Ordinal))
    {
throw new InvalidOperationException(std::string("Packaged asset path '") + path + std::string("' is not canonical. Expected '") + normalizedPath + std::string("'."));
    }
return normalizedPath;}

std::string CanonicalPackagedAssetPath::CollapseAndValidateSegments(std::string normalizedPath, std::string originalPath)
{
StringBuilder *builder = new StringBuilder(static_cast<int32_t>(static_cast<int32_t>(normalizedPath.size())));
auto __localDeleteGuard_00000039 = he_cpp_make_scope_exit([&]() {
delete builder;
});
bool wroteSegment = false;
int32_t index = 0;
while (index < static_cast<int32_t>(normalizedPath.size())) {
while (index < static_cast<int32_t>(normalizedPath.size()) && normalizedPath[index] == '/') {
index++;
}
    if (index >= static_cast<int32_t>(normalizedPath.size()))
    {
break;
    }
const int32_t segmentStart = index;
while (index < static_cast<int32_t>(normalizedPath.size()) && normalizedPath[index] != '/') {
index++;
}
const std::string segment = String::Substring(normalizedPath, segmentStart, index - segmentStart);
    if (segment == "." || segment == "..")
    {
throw new InvalidOperationException(std::string("Packaged asset path '") + originalPath + std::string("' must not contain traversal segments."));
    }
    if (wroteSegment)
    {
builder->Append(static_cast<char>('/'));
    }
builder->Append(segment);
wroteSegment = true;
}
return builder->ToString();}

bool CanonicalPackagedAssetPath::IsRootedPath(std::string path)
{
    if (static_cast<int32_t>(path.size()) == 0)
    {
return false;    }
    if (path[0] == '/')
    {
return true;    }
return static_cast<int32_t>(path.size()) >= 2 && path[1] == ':';}

std::string CanonicalPackagedAssetPath::NormalizeSlashAndCase(std::string path)
{
const std::string lowercasePath = String::ToLowerInvariant(path);
StringBuilder *builder = new StringBuilder(static_cast<int32_t>(static_cast<int32_t>(lowercasePath.size())));
auto __localDeleteGuard_0000003A = he_cpp_make_scope_exit([&]() {
delete builder;
});
for (int32_t index = 0; index < static_cast<int32_t>(lowercasePath.size()); index++) {
const char character = lowercasePath[index];
    if (character == '\\')
    {
builder->Append(static_cast<char>('/'));
continue;
    }
builder->Append(static_cast<char>(character));
}
return builder->ToString();}

