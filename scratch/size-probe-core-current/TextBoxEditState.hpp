#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class TextBoxEditState
{
public:
    virtual ~TextBoxEditState() = default;

    const std::string& get_Text();

    void set_Text(std::string value);

    int32_t get_CursorPosition();

    void set_CursorPosition(int32_t value);

    int32_t get_SelectionAnchorPosition();

    bool get_HasSelection();

    int32_t get_SelectionStart();

    int32_t get_SelectionEnd();

    void Backspace();

    void ClearSelection();

    void Delete();

    std::string GetSelectedText();

    void InsertCharacter(char character);

    void InsertText(std::string text);

    void MoveCursorLeft();

    void MoveCursorRight();

    TextBoxEditState();

    TextBoxEditState(std::string text);

    void SelectAll();

    void SetCursorToEnd();

    void SetCursorToStart();

    void SetSelection(int32_t anchorPosition, int32_t cursorPosition);
private:
    std::string TextValue;

    int32_t CursorPositionValue;

    int32_t SelectionAnchorPositionValue;

    int32_t ClampCursor(int32_t value);

    bool RemoveSelection();
};
