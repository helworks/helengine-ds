#ifdef DrawText
#undef DrawText
#endif
#include "SceneIdUtility.hpp"
#include "runtime/native_string.hpp"
#include "system/io/path.hpp"
#include "runtime/native_string.hpp"
#include "system/io/path.hpp"

std::string SceneIdUtility::FromPath(std::string scenePath)
{
    if (String::IsNullOrWhiteSpace(scenePath))
    {
return String::Empty;    }
const std::string sceneFileName = Path::GetFileName(scenePath);
    if (String::IsNullOrWhiteSpace(sceneFileName))
    {
return String::Empty;    }
const std::string sceneExtension = ".helen";
    if (!String::EndsWith(sceneFileName, sceneExtension, StringComparison::OrdinalIgnoreCase))
    {
return sceneFileName;    }
return String::Substring(sceneFileName, 0, static_cast<int32_t>(sceneFileName.size()) - 6);}

