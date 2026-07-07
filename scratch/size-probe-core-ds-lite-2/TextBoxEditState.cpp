#ifdef DrawText
#undef DrawText
#endif
#include "TextBoxEditState.hpp"
#include "runtime/native_string.hpp"
#include "TextBoxEditState.hpp"
#include "system/math.hpp"
#include "system/math.hpp"
#include "runtime/native_string.hpp"

const std::string& TextBoxEditState::get_Text()
{
return this->TextValue;}

void TextBoxEditState::set_Text(std::string value)
{
this->TextValue = value;
this->CursorPositionValue = this->ClampCursor(static_cast<int32_t>(this->CursorPositionValue));
this->SelectionAnchorPositionValue = this->ClampCursor(static_cast<int32_t>(this->SelectionAnchorPositionValue));
}

int32_t TextBoxEditState::get_CursorPosition()
{
return this->CursorPositionValue;}

void TextBoxEditState::set_CursorPosition(int32_t value)
{
this->CursorPositionValue = this->ClampCursor(static_cast<int32_t>(value));
this->SelectionAnchorPositionValue = this->CursorPositionValue;
}

int32_t TextBoxEditState::get_SelectionAnchorPosition()
{
return this->SelectionAnchorPositionValue;}

bool TextBoxEditState::get_HasSelection()
{
return this->SelectionAnchorPositionValue != this->CursorPositionValue;}

int32_t TextBoxEditState::get_SelectionStart()
{
return Math::Min(static_cast<int32_t>(this->SelectionAnchorPositionValue), static_cast<int32_t>(this->CursorPositionValue));}

int32_t TextBoxEditState::get_SelectionEnd()
{
return Math::Max(static_cast<int32_t>(this->SelectionAnchorPositionValue), static_cast<int32_t>(this->CursorPositionValue));}

void TextBoxEditState::Backspace()
{
    if (this->RemoveSelection())
    {
return;    }
    if (this->CursorPositionValue <= 0)
    {
return;    }
this->TextValue = String::Remove(this->TextValue, this->CursorPositionValue - 1, 1);
this->CursorPositionValue--;
this->SelectionAnchorPositionValue = this->CursorPositionValue;
}

void TextBoxEditState::ClearSelection()
{
this->SelectionAnchorPositionValue = this->CursorPositionValue;
}

void TextBoxEditState::Delete()
{
    if (this->RemoveSelection())
    {
return;    }
    if (this->CursorPositionValue < 0 || this->CursorPositionValue >= static_cast<int32_t>(this->TextValue.size()))
    {
return;    }
this->TextValue = String::Remove(this->TextValue, this->CursorPositionValue, 1);
this->CursorPositionValue = this->ClampCursor(static_cast<int32_t>(this->CursorPositionValue));
this->SelectionAnchorPositionValue = this->CursorPositionValue;
}

std::string TextBoxEditState::GetSelectedText()
{
    if (!this->get_HasSelection())
    {
return String::Empty;    }
return String::Substring(this->TextValue, this->get_SelectionStart(), this->get_SelectionEnd() - this->get_SelectionStart());}

void TextBoxEditState::InsertCharacter(char character)
{
this->RemoveSelection();
const int32_t insertionIndex = this->ClampCursor(static_cast<int32_t>(this->CursorPositionValue));
this->TextValue = String::Insert(this->TextValue, insertionIndex, std::string(1, character));
this->CursorPositionValue = insertionIndex + 1;
this->SelectionAnchorPositionValue = this->CursorPositionValue;
}

void TextBoxEditState::InsertText(std::string text)
{
const std::string textValue = text;
this->RemoveSelection();
const int32_t insertionIndex = this->ClampCursor(static_cast<int32_t>(this->CursorPositionValue));
this->TextValue = String::Insert(this->TextValue, insertionIndex, textValue);
this->CursorPositionValue = insertionIndex + static_cast<int32_t>(textValue.size());
this->SelectionAnchorPositionValue = this->CursorPositionValue;
}

void TextBoxEditState::MoveCursorLeft()
{
    if (this->get_HasSelection())
    {
this->CursorPositionValue = this->get_SelectionStart();
this->SelectionAnchorPositionValue = this->CursorPositionValue;
return;    }
this->CursorPositionValue = Math::Max(static_cast<int32_t>(0), static_cast<int32_t>(this->CursorPositionValue - 1));
this->SelectionAnchorPositionValue = this->CursorPositionValue;
}

void TextBoxEditState::MoveCursorRight()
{
    if (this->get_HasSelection())
    {
this->CursorPositionValue = this->get_SelectionEnd();
this->SelectionAnchorPositionValue = this->CursorPositionValue;
return;    }
this->CursorPositionValue = Math::Min(static_cast<int32_t>(static_cast<int32_t>(this->TextValue.size())), static_cast<int32_t>(this->CursorPositionValue + 1));
this->SelectionAnchorPositionValue = this->CursorPositionValue;
}

TextBoxEditState::TextBoxEditState() : TextValue(""), CursorPositionValue(0), SelectionAnchorPositionValue(0)
{
}

TextBoxEditState::TextBoxEditState(std::string text) : TextValue(""), CursorPositionValue(0), SelectionAnchorPositionValue(0)
{
this->set_Text(text);
this->SetCursorToEnd();
}

void TextBoxEditState::SelectAll()
{
this->SelectionAnchorPositionValue = 0;
this->CursorPositionValue = static_cast<int32_t>(this->TextValue.size());
}

void TextBoxEditState::SetCursorToEnd()
{
this->CursorPositionValue = static_cast<int32_t>(this->TextValue.size());
this->SelectionAnchorPositionValue = this->CursorPositionValue;
}

void TextBoxEditState::SetCursorToStart()
{
this->CursorPositionValue = 0;
this->SelectionAnchorPositionValue = 0;
}

void TextBoxEditState::SetSelection(int32_t anchorPosition, int32_t cursorPosition)
{
this->SelectionAnchorPositionValue = this->ClampCursor(static_cast<int32_t>(anchorPosition));
this->CursorPositionValue = this->ClampCursor(static_cast<int32_t>(cursorPosition));
}

int32_t TextBoxEditState::ClampCursor(int32_t value)
{
return Math::Max(static_cast<int32_t>(0), static_cast<int32_t>(Math::Min(static_cast<int32_t>(value), static_cast<int32_t>(static_cast<int32_t>(this->TextValue.size())))));}

bool TextBoxEditState::RemoveSelection()
{
    if (!this->get_HasSelection())
    {
return false;    }
const int32_t selectionStart = this->get_SelectionStart();
const int32_t selectionLength = this->get_SelectionEnd() - selectionStart;
this->TextValue = String::Remove(this->TextValue, selectionStart, selectionLength);
this->CursorPositionValue = selectionStart;
this->SelectionAnchorPositionValue = selectionStart;
return true;}

