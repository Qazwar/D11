#pragma once
#include "..\renderer\render_types.h"
#include "..\lib\collection_types.h"
#include "..\sprites\Sprite.h"
#include "..\renderer\BitmapFont.h"

namespace ds {

	namespace font {

		v2 calculateSize(RID bitmapFont, const char* text, int padding = 4, float scaleX = 1.0f, float scaleY = 1.0f);

		v2 calculateLimitedSize(RID bitmapFont, const char* text, int chars, int padding = 4, float scaleX = 1.0f, float scaleY = 1.0f);

		//void createText(RID bitmapFont, const v2& pos, const char* text, const Color& color, Array<Sprite>& sprites, float scaleX = 1.0f, float scaleY = 1.0f);

		//void createCharacter(RID bitmapFont, const v2& pos, char c, const Color& color, Array<Sprite>& sprites, float scaleX = 1.0f, float scaleY = 1.0f);
		
	}
}

