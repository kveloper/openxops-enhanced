//! @file gui-object.cpp
//! @brief Definitions of GUI object classes.

//--------------------------------------------------------------------------------
// 
// OpenXOPS
// Copyright (c) 2014-2023, OpenXOPS Project / [-_-;](mikan) All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright notice, 
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice, 
//   this list of conditions and the following disclaimer in the documentation 
//   and/or other materials provided with the distribution.
// * Neither the name of the OpenXOPS Project nor the names of its contributors 
//   may be used to endorse or promote products derived from this software 
//   without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL OpenXOPS Project BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//--------------------------------------------------------------------------------

#include "gui-object.h"

//! @brief Constructor
ScrollbarObject::ScrollbarObject()
{
	pos_x = 0;
	pos_y = 0;
	width = 0;
	height = 0;
	totalitems = 0;
	showtems = 0;
	scrollitem = 0;
	knob_height = 0.0f;
	knob_scale = 0.0f;
	knob_y = 0;
	mouse_mode = 0;
	color_body = 0x00000000;
	color1_default = 0x00000000;
	color2_default = 0x00000000;
	color1_cursor = 0x00000000;
	color2_cursor = 0x00000000;
	color1_select = 0x00000000;
	color2_select = 0x00000000;
}

//! @brief Destructor
ScrollbarObject::~ScrollbarObject()
{
	//
}

//! @brief Scroll Bar Initialization
//! @param x x Coordinate
//! @param y y coordinates
//! @param w Width
//! @param h Height.
//! @param in_totalitems Total Items
//! @param in_showtems Number of items displayed on the screen
//! @param in_scrollitem Number of scrolled items
//! @warning Set the size to 8 or more in width and height.
//! @attention Set the scrollbar colors with SetColor().
void ScrollbarObject::Create(int x, int y, int w, int h, int in_totalitems, int in_showtems, int in_scrollitem)
{
	pos_x = x;
	pos_y = y;
	width = w;
	height = h;
	totalitems = in_totalitems;
	showtems = in_showtems;
	scrollitem = in_scrollitem;
	if( totalitems < showtems ){
		showtems = totalitems;
		knob_height = (float)h;
		knob_scale = 1.0f;
	}
	else{
		knob_height = (float)h / totalitems * showtems;
		knob_scale = ((float)h - knob_height) / (totalitems - showtems);
	}
	knob_y = (int)Round(knob_scale * scrollitem);
	mouse_mode = 0;
}

//! @brief Set the color of the scrollbar
//! @param body Color of body (back side)
//! @param default1 Normal color (outside)
//! @param default2 Normal color (inside)
//! @param cursor1 Cursor color (outside)
//! @param cursor2 Cursor selection color (inside)
//! @param select1 Outer knob color when clicked.
//! @param select2 Inner knob color when clicked.
void ScrollbarObject::SetColor(int body, int default1, int default2, int cursor1, int cursor2, int select1, int select2)
{
	color_body = body;
	color1_default = default1;
	color2_default = default2;
	color1_cursor = cursor1;
	color2_cursor = cursor2;
	color1_select = select1;
	color2_select = select2;
}

//! @brief Number of scrolled items
//! @param id Number of items
void ScrollbarObject::SetScrollItem(int id)
{
	if( (id < 0)||(totalitems <= id) ){ return; }

	scrollitem = id;
}

//! @brief Get the number of scrolled items
//! @return Number of items
int ScrollbarObject::GetScrollItem()
{
	return scrollitem;
}

//! @brief Scroll to top
void ScrollbarObject::ScrollUP()
{
	if( scrollitem > 0 ){
		scrollitem -= 1;
	}
}

//! @brief Scroll down
void ScrollbarObject::ScrollDOWN()
{
	if( scrollitem < (totalitems - showtems) ){
		scrollitem += 1;
	}
}

//! @brief Scrolling or Determining
//! @return scrolling: true Not scrolling: false
bool ScrollbarObject::CheckScrolling()
{
	if( mouse_mode == 2 ){ return true; }
	return false;
}

//! @brief Scroll bar input processing
//! @param mouse_x Mouse x Coordinate
//! @param mouse_y Mouse y coordinates
//! @param mouse_click Mouse click flag
//! @param scroll_y y-directional scrolling
void ScrollbarObject::Input(int mouse_x, int mouse_y, bool mouse_click, int scroll_y)
{
	//mouse_mode = 0: mouse cursor is outside the scrollbar area.
	//mouse_mode = 1: mouse cursor is over the scrollbar.
	//mouse_mode = 2: mouse cursor is clicking the scrollbar.

	//Determining the operation of the scrollbar
	if( mouse_mode == 2 ){
		if( mouse_click == false ){
			mouse_mode = 0;	//or 1
		}
	}
	if( mouse_mode != 2 ){
		if( (pos_x < mouse_x)&&(mouse_x < (pos_x+width))
			&&(pos_y + (int)(knob_scale * scrollitem) < mouse_y)&&(mouse_y < pos_y + (int)(knob_scale * scrollitem + knob_height))
		){
			if( mouse_click == true ){
				mouse_mode = 2;
			}
			else{
				mouse_mode = 1;
			}
		}
		else{
			mouse_mode = 0;
		}
	}

	//Moving scrollbar
	if( mouse_mode == 2 ){
		knob_y += scroll_y;
		if( knob_y < 0 ){ knob_y = 0; }
		if( knob_y > (height - (int)knob_height) ){ knob_y = (height - (int)knob_height); }

		scrollitem = (int)Round((float)knob_y / knob_scale);
		if( scrollitem < 0 ){ scrollitem = 0; }
		if( scrollitem > (totalitems - showtems) ){ scrollitem = (totalitems - showtems); }
	}
	else{
		knob_y = (int)Round(knob_scale * scrollitem);
	}
}

//! @brief Scroll Bar Drawing Process
//! @param d3dg Pointer to D3DGraphics.
//! @attention In the case of "Total item number <Number of items to be displayed", the knob is not displayed.
void ScrollbarObject::Draw(class D3DGraphics *d3dg)
{
	if( d3dg == NULL ){ return; }

	int color, color2;

	//Area drawing
	d3dg->Draw2DBox(pos_x, pos_y, pos_x+width, pos_y+height, color_body);

	//Render the knob.
	if( totalitems > showtems ){
		//Set Color
		if( mouse_mode == 2 ){
			color = color1_select;
			color2 = color2_select;
		}
		else if( mouse_mode == 1 ){
			color = color1_cursor;
			color2 = color2_cursor;
		}
		else{
			color = color1_default;
			color2 = color2_default;
		}

		//Render
		d3dg->Draw2DBox(pos_x, pos_y+knob_y, pos_x+width, pos_y+knob_y + (int)knob_height, color);
		d3dg->Draw2DBox(pos_x+3, pos_y+knob_y+3, pos_x+width-3, pos_y+knob_y + (int)knob_height -3, color2);
	}
}

//! @brief Constructor
TextBoxObject::TextBoxObject()
{
	mode = 0;
	inputstr[0] = '\0';
	maxlen = MAX_INPUTBOXLEN;
}

//! @brief Destructor
TextBoxObject::~TextBoxObject()
{}

//! @brief Initialization of input box
//! @param str Default String
//! @param in_maxlen Maximum number of characters (1-127)
//! @param in_mode Input mode
//! @note The input modeifies the value of the following values:
//! @note 0x01: Number, 0x02: Number/-/., 0x04: Upper lowercase, 0x08: Upper uppercase, 0x20: Space
void TextBoxObject::InitTextBox(const char *str, int in_maxlen, int in_mode)
{
	if( in_maxlen < 1 ){ in_maxlen = 1; }
	if( in_maxlen > MAX_INPUTBOXLEN ){ in_maxlen = MAX_INPUTBOXLEN; }

	mode = in_mode;
	SetTextBoxStr(str);
	maxlen = in_maxlen;
}

//! @brief Input box release
void TextBoxObject::DestroyTextBox()
{}

//! @brief Set the input box string
//! @param str String.
void TextBoxObject::SetTextBoxStr(const char *str)
{
	if( str == NULL ){ str = ""; }
	if( (int)strlen((char*)str) > maxlen ){ return; }

	strcpy(inputstr, (char*)str);
}

//! @brief Get the input box string
//! @return String Pointer
const char* TextBoxObject::GetTextBoxStr()
{
	return inputstr;
}

//! @brief Add a single character
//! @param inchar Adding Characters
void TextBoxObject::InputChar(char inchar)
{
	for(int i=0; i<maxlen; i++){
		if( inputstr[i] == '\0' ){
			inputstr[i] = inchar;
			inputstr[i+1] = '\0';
			return;
		}
	}
}

//! @brief Deleting a character
void TextBoxObject::DeleteChar()
{
	int s = (int)strlen(inputstr);
	if( s == 0 ){ return; }
	inputstr[ s-1 ] = '\0';
}

//! @brief Input box string processing
//! @param inputCtrl Pointer of InputControl class
//! @return ENTER pressed after input: true; otherwise (including errors): false.
//! @attention InputControl must be processed appropriately beforehand; for example, by calling GetInputState().
bool TextBoxObject::ProcessTextBox(InputControl *inputCtrl)
{
	if( inputCtrl == NULL ){ return false; }

	if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x0F)) ){		// [ENTER]
		//If you have any questions,
		if( inputstr[0] != '\0' ){
			return true;
		}
	}
	else{
		//Removing a character
		if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x0E)) ){		// [BACK]
			DeleteChar();
		}

		if( (mode & 0x01)||(mode & 0x02) ){
			//Number.
			for(char key=0x16; key<=0x1F; key++){
				if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(key)) ){
					InputChar(key + 0x1A);
				}
			}
			for(char key=0x04; key<=0x0D; key++){
				if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(key)) ){
					InputChar(key + 0x2C);
				}
			}

			if( mode & 0x02 ){
				//if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x48)) ){ InputChar('+'); }
				if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x3D)) ){ InputChar('-'); }
				if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x49)) ){ InputChar('-'); }
				if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x44)) ){ InputChar('.'); }
				if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x4A)) ){ InputChar('.'); }
			}
		}

		if( (mode & 0x04)&&(mode & 0x08) ){
			if( inputCtrl->CheckKeyNow(OriginalkeycodeToDinputdef(0x14)) ){	// [SHIFT]
				//Upper case
				for(char key=0x20; key<=0x39; key++){
					if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(key)) ){
						InputChar(key + 0x21);
					}
				}
			}
			else{
				//lowercase
				for(char key=0x20; key<=0x39; key++){
					if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(key)) ){
						InputChar(key + 0x41);
					}
				}
			}
		}
		else if( mode & 0x04 ){
			//lowercase
			for(char key=0x20; key<=0x39; key++){
				if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(key)) ){
					InputChar(key + 0x41);
				}
			}
		}
		else if( mode & 0x08 ){
			//Upper case
			for(char key=0x20; key<=0x39; key++){
				if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(key)) ){
					InputChar(key + 0x21);
				}
			}
		}

		//if( mode & 0x10 ){
		//	if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x43)) ){ InputChar(','); }
		//}

		if( mode & 0x20 ){
			//Space
			if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x11)) ){		// [SPACE]
				InputChar(' ');
			}
		}
	}

	return false;
}
