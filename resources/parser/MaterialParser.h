#pragma once
#include "ResourceParser.h"
/*
material {
	name : "RTMtrl"
	diffuse : 255,255,255,255
	ambient : 32,32,32,2555
	blend_state : "DefaultBlendState"
	shader : "SpriteShader"
	render_target : "RT1"
}
*/
namespace ds {

	namespace res {

		class MaterialParser : public ResourceParser {

		public:
			MaterialParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
			RID createMaterial(const char* name, const MaterialDescriptor& descriptor);		
		};

	}

}