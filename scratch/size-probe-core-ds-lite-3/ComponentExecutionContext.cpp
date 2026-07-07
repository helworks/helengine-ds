#ifdef DrawText
#undef DrawText
#endif
#include "ComponentExecutionContext.hpp"
#include "runtime/native_exceptions.hpp"
#include "ComponentExecutionMode.hpp"
#include "ComponentExecutionContext.hpp"
#include "runtime/native_exceptions.hpp"

::ComponentExecutionMode ComponentExecutionContext::get_CurrentMode()
{
    if (EditorExecutionDepth > 0)
    {
return ComponentExecutionMode::Editor;    }
return ComponentExecutionMode::Runtime;}

void ComponentExecutionContext::EnterEditor()
{
EditorExecutionDepth++;
}

void ComponentExecutionContext::ExitEditor()
{
    if (EditorExecutionDepth <= 0)
    {
throw new InvalidOperationException("Editor component execution scope was exited without a matching entry.");
    }
EditorExecutionDepth--;
}

int32_t ComponentExecutionContext::EditorExecutionDepth = 0;

