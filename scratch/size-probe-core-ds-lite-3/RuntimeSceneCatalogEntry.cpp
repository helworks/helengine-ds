#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeSceneCatalogEntry.hpp"
#include "runtime/native_string.hpp"
#include "RuntimeSceneCatalogEntry.hpp"
#include "CanonicalPackagedAssetPath.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

const std::string& RuntimeSceneCatalogEntry::get_SceneId()
{
return this->SceneId;
}

const std::string& RuntimeSceneCatalogEntry::get_CookedRelativePath()
{
return this->CookedRelativePath;
}

RuntimeSceneCatalogEntry::RuntimeSceneCatalogEntry(std::string sceneId, std::string cookedRelativePath) : SceneId(), CookedRelativePath()
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_00000144 = "Scene id is required.";
auto __ctor_arg_00000145 = "sceneId";
return new ArgumentException(__ctor_arg_00000144, __ctor_arg_00000145);
})();
    }
    if (String::IsNullOrWhiteSpace(cookedRelativePath))
    {
throw ([&]() {
auto __ctor_arg_00000146 = "Cooked relative path is required.";
auto __ctor_arg_00000147 = "cookedRelativePath";
return new ArgumentException(__ctor_arg_00000146, __ctor_arg_00000147);
})();
    }
this->SceneId = sceneId;
this->CookedRelativePath = RuntimeSceneCatalogEntry::NormalizeCookedRelativePath(cookedRelativePath);
}

bool RuntimeSceneCatalogEntry::IsRootedRuntimePath(std::string path)
{
    if (String::IsNullOrWhiteSpace(path))
    {
return false;    }
    if (path[0] == '/' || path[0] == '\\')
    {
return true;    }
    if (static_cast<int32_t>(path.size()) >= 2 && path[1] == ':')
    {
return true;    }
for (int32_t index = 1; index < static_cast<int32_t>(path.size()) - 1; index++) {
    if (path[index] != ':')
    {
continue;
    }
const char nextCharacter = path[index + 1];
return nextCharacter == '/' || nextCharacter == '\\';}
return false;}

std::string RuntimeSceneCatalogEntry::NormalizeCookedRelativePath(std::string cookedRelativePath)
{
    if (RuntimeSceneCatalogEntry::IsRootedRuntimePath(cookedRelativePath))
    {
return cookedRelativePath;    }
return CanonicalPackagedAssetPath::ValidateCanonical(cookedRelativePath);}

