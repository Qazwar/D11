#include "font.h"
#include "..\utils\Log.h"
#include "..\resources\ResourceContainer.h"

namespace ds {

	namespace font {
	
		// -------------------------------------------------------
		// Calculate size of text
		// -------------------------------------------------------
		v2 calculateSize(RID bitmapFont, const char* text, int padding, float scaleX, float scaleY) {
			float cPadding = 0.0f;
			v2 ret(0.0f, 0.0f);
			int len = strlen(text);
			Bitmapfont* font = res::getFont(bitmapFont);
			for (int cnt = 0; cnt < len; ++cnt) {
				char c = text[cnt];
				const Texture& t = font->get(c);
				cPadding = (t.rect.width() + padding)  * scaleX;
				float dimX = t.rect.width() * scaleX;
				ret.x += dimX + padding;
				float h = t.rect.height() * scaleY;
				if (h > ret.y) {
					ret.y = h;
				}
			}
			return ret;
		}

		// -------------------------------------------------------
		// Calculate limited size of text
		// -------------------------------------------------------
		v2 calculateLimitedSize(RID bitmapFont, const char* text, int chars, int padding, float scaleX, float scaleY) {
			float cPadding = 0.0f;
			v2 ret(0.0f, 0.0f);
			int len = strlen(text);
			if (len > chars) {
				len = chars;
			}
			Bitmapfont* font = res::getFont(bitmapFont);
			for (int cnt = 0; cnt < len; ++cnt) {
				char c = text[cnt];
				const Texture& t = font->get(c);
				cPadding = (t.rect.width() + padding)  * scaleX;
				float dimX = t.rect.width() * scaleX;
				ret.x += dimX + padding;
				float h = t.rect.height() * scaleY;
				if (h > ret.y) {
					ret.y = h;
				}
			}
			return ret;
		}
		/*
		// -------------------------------------------------------
		// Create text
		// -------------------------------------------------------
		void createText(RID bitmapFont, const v2& pos, const char* text, const Color& color, Array<Sprite>& sprites, float scaleX, float scaleY) {
			float x = pos.x;
			float y = pos.y;
			float padding = 0.0f;
			float z = 0.0f;			
			int len = strlen(text);
			for (int cnt = 0; cnt < len; ++cnt) {
				char c = text[cnt];
				if (bitmapFont.contains(c)) {
					CharDef cd = bitmapFont.getCharDef(c);
					padding = (cd.width + 2)  * scaleX;
					float dimX = cd.width * scaleX;
					float dimY = bitmapFont.charHeight * scaleY;
					// quad buffer is centered by default so adjust position!
					Sprite sp;
					sp.position = (Vector2f(x + dimX * 0.5f, y + dimY * 0.5f));
					sp.texture = cd.texture;// math::buildTexture(cd.texureRect);
					sp.color = color;
					sp.scale = Vector2f(scaleX, scaleY);
					sprites.push_back(sp);
					x += dimX + 4;
				}
			}
		}

		void createCharacter(RID bitmapFont, const v2& pos, char c, const Color& color, Array<Sprite>& sprites, float scaleX, float scaleY) {
			float x = pos.x;
			float y = pos.y;
			float padding = 0.0f;
			float z = 0.0f;
			CharDef cd = bitmapFont.getCharDef(c);
			padding = (cd.width + 2)  * scaleX;
			float dimX = cd.width * scaleX;
			float dimY = bitmapFont.charHeight * scaleY;
			// quad buffer is centered by default so adjust position!
			Sprite sp;
			sp.position = (Vector2f(x + dimX * 0.5f, y + dimY * 0.5f));
			sp.texture = cd.texture;// math::buildTexture(cd.texureRect);
			sp.color = color;
			sp.scale = Vector2f(scaleX, scaleY);
			sprites.push_back(sp);
				
		}
		*/
	}
}