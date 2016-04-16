#pragma once
#include <Vector.h>
#include "..\utils\Color.h"
#include "..\renderer\render_types.h"
#include "..\lib\collection_types.h"
#include <map>
#include "..\sprites\Sprite.h"
#include "..\sprites\SpriteArray.h"
#include "..\physics\ColliderArray.h"
#include "..\physics\PhysicalWorld.h"
#include "..\math\tweening.h"
#include "..\math\CubicBezierPath.h"
#include "..\math\StraightPath.h"
#include "..\math\FloatArray.h"
#include "..\io\DataFile.h"
#include "AdditionalData.h"
#include "..\resources\ResourceDescriptors.h"
#include "..\renderer\sprites.h"

namespace ds {

	enum BounceDirection {
		BD_X,
		BD_Y,
		BD_BOTH,
		BD_EOL
	};

	enum ActionType {
		AT_ALPHA_FADE_TO,
		AT_COLOR_FADE_TO,
		AT_MOVE_TO,
		AT_MOVE_BY,
		AT_FOLLOW_PATH,
		AT_FOLLOW_CURVE,
		AT_FOLLOW_TARGET,
		AT_REMOVE_AFTER,
		AT_WAIT,
		AT_MOVE_WITH,
		AT_ROTATE,
		AT_ROTATE_BY,
		AT_FOLLOW_STRAIGHT_PATH,
		AT_KILL,
		AT_SCALE_BY_PATH,
		AT_SCALE,
		AT_COLOR_FLASH
	};

	enum RepeatType {
		RT_ONCE,
		RT_FORVER,
		RT_REPEATED
	};

	class AbstractAction;
	class MoveToAction;
	class ScalingAction;
	class AlphaFadeToAction;
	class RotateByAction;
	class FollowPathAction;
	class FollowCurveAction;
	class MoveByAction;
	class WaitAction;
	class MoveWithAction;
	class RotateAction;
	class FollowTargetAction;
	class FollowStraightPathAction;
	class ColorFadeToAction;
	class RemoveAfterAction;
	class ScaleByPathAction;
	class ColorFlashAction;
	class BehaviorDefinitions;

	typedef void (*MoveFunc)(v2&,float*,float);

	struct ActionEvent {

		SID sid;
		ActionType type;
		int spriteType;
	};

	struct ActionEventBuffer {

		Array<ActionEvent> events;

		void reset() {
			events.clear();
		}

		void add(SID sid,ActionType type,int spriteType) {
			ActionEvent e;
			e.sid = sid;
			e.type = type;
			e.spriteType = spriteType;
			events.push_back(e);
		}
	};

	

	struct WorldLayer {

		bool visible;
		int descriptor_id;

		WorldLayer() : visible(true), descriptor_id(0) {}
	};

	class World {

	typedef Array<AbstractAction*> Actions;

	public:
		World(const WorldDescriptor& descriptor);
		~World(void);
		SID create(const v2& pos, const Texture& r, int type = -1, int layer = 0);
		SID create(const v2& pos, const char* templateName, int layer = 0);
		SID create(const v2& pos, const Texture& r,float rotation, float scaleX, float scaleY,const Color& color = Color::WHITE,int type = -1, int layer = 0);
		void remove(SID sid);
		void render();
		//void renderLayers(int* layers,int num_layers);
		//void renderSingleLayer(int layer);
		void setDefaultDescriptor(int id);
		void setBoundingRect(const Rect& r);

		const bool contains(SID sid) const {
			return m_Data.contains(sid);
		}
		void tick(float dt);

		//
		void setPosition(SID sid,const v2& pos) {
			m_Data.setPosition(sid,pos);
		}
		void setTexture(SID sid,const Texture& t) {
			SpriteArrayIndex &in = m_Data.indices[sid];
			assert(in.index != USHRT_MAX);
			m_Data.textures[in.index] = t;
		}
		const Texture& getTexture(SID sid) const {
			SpriteArrayIndex &in = m_Data.indices[sid];
			assert(in.index != USHRT_MAX);
			return m_Data.textures[in.index];
		}
		// scaling
		void scale(SID sid,float sx,float sy) {
			m_Data.setScale(sid,sx,sy);
		}
		void scale(int index,float sx,float sy) {
			m_Data.scales[index] = v2(sx,sy);
		}
		void scaleTo(SID sid, const v2& startScale, const v2& endScale, float ttl, int mode = 0, const tweening::TweeningType& tweeningType = &tweening::linear);
		void scaleByPath(SID id, Vector2fPath* path, float ttl);
		void clear();

		const int size() const {
			return m_Data.num;
		}

		void moveTo(SID sid,const v2& startPos,const v2& endPos,float ttl,int mode = 0,const tweening::TweeningType& tweeningType = &tweening::linear);
		const int getMovingNumber() const;
		void moveBy(SID sid,const v2& velocity,bool bounce = false);
		void bounce(SID sid, BounceDirection direction,float dt);
		const v2& getPosition(SID sid) const {
			return m_Data.getPosition(sid);
		}
		void moveWith(SID sid,const MoveFunc& function,float ttl,int mode = 0);


		float getRotation(SID sid) const;
		void setRotation(SID sid,float angle);
		void setRotation(SID sid, const v2& target);
		void rotateBy(SID sid, float angle, float ttl, int mode = 0, const tweening::TweeningType& tweeningType = &tweening::linear);
		void rotateTo(SID sid, float angle, float ttl, int mode = 0, const tweening::TweeningType& tweeningType = &tweening::linear);
		void rotateTo(SID sid, const v2& target, float ttl, const tweening::TweeningType& tweeningType = &tweening::linear);
		void rotate(SID sid, float angleVelocity, float ttl,int mode = 0);

		void removeAfter(SID sid, float ttl);

		void attachTo(SID sid, SID parent);

		void setColor(SID sid,const Color& clr) {
			m_Data.setColor(sid,clr);
		}

		void followPath(SID sid,CubicBezierPath* path,float ttl,int mode = 0);

		void followPath(SID sid, StraightPath* path, float ttl, int mode = 0);

		void followCurve(SID sid,BezierCurve* path,float ttl,int mode = 0);

		void followTarget(SID sid,SID target, float velocity);

		void fadeAlphaTo(SID sid, float startAlpha, float endAlpha, float ttl, int mode = 0, const tweening::TweeningType& tweeningType = &tweening::linear);

		void fadeColorTo(SID sid, const Color& startColor, const Color& endColor, float ttl, int mode = 0, const tweening::TweeningType& tweeningType = &tweening::linear);

		void flashColor(SID sid, const Color& startColor, const Color& endColor, float ttl, int mode = 0, const tweening::TweeningType& tweeningType = &tweening::linear);
		
		void wait(SID sid,float ttl);

		void debug();

		void save(const ReportWriter& writer);

		void debug(SID sid);

		void stopAction(SID sid, ActionType type);

		const ActionEventBuffer& getEventBuffer() const {
			return m_Buffer;
		}

		bool hasEvents() const {
			return m_Buffer.events.size() > 0;
		}
		int getType(SID sid) const;

		void setType(SID sid,int type);

		int find_by_type(int type, SID* ids, int max) const;

		int find_by_types(const int* types, int num, SID* ids, int max) const;

		int get_count(int type);

		void attachCollider(SID sid, const v2& extent) {
			m_Data.attachCollider(sid, extent);
		}

		void attachCollider(SID sid){
			m_Data.attachCollider(sid);
		}

		void attachBoxCollider(SID sid){
			m_Data.attachCollider(sid, SST_BOX);
		}

		void ignoreCollisions(int firstType, int secondType) {
			m_Physics.ignore(firstType, secondType);
		}

		void ignoreLayer(int layer) {
			m_Physics.ignoreLayer(layer);
		}

		bool hasCollisions() {
			return m_Physics.hasCollisions();
		}
		int getNumCollisions() {
			return m_Physics.getNumCollisions();
		}
		const Collision& getCollision(int idx) const {
			return m_Physics.getCollision(idx);
		}

		void drawColliders(const Texture& texture) {
			m_Physics.drawColliders(texture);
		}

		SID pick(const v2& pos);

		const SpriteArray& getSpriteArray() const {
			return m_Data;
		}

		void* attach_data(SID sid,int size, int identifier) {
			return _buffer.attach(sid, size, identifier);
		}

		void* get_data(SID sid) {
			return _buffer.get(sid);
		}


		void attach_descriptor(int layer, const char* descName);

		void activateLayer(int layer);

		void deactivateLayer(int layer);

		void loadBehaviors();

		AbstractAction* find_action(const char* name) const;

		void startBehavior(SID id, const char* name);

		void enableCollisionChecks() {
			_checkCollisions = true;
		}
	private:
		ActionEventBuffer m_Buffer;
		//void allocate(int size);
		AbstractAction* find_action(ActionType type) const;

		WorldLayer _layers[32];

		SpriteArray m_Data;
		SpriteBuffer* _spriteBuffer;
		AdditionalData _buffer;
		bool _checkCollisions;
		PhysicalWorld m_Physics;

		MoveToAction* m_MoveToAction;	
		ScalingAction* m_ScalingAction;
		AlphaFadeToAction* m_AlphaFadeToAction;
		RotateByAction* m_RotateByAction;
		FollowPathAction* m_FollowPathAction;
		FollowCurveAction* m_FollowCurveAction;
		MoveByAction* m_MoveByAction;
		WaitAction* m_WaitAction;
		MoveWithAction* m_MoveWithAction;
		RotateAction* m_RotateAction;
		FollowTargetAction* m_FollowTargetAction;
		FollowStraightPathAction* m_FollowStraightPathAction;
		ColorFadeToAction* m_ColorFadeToAction;
		RemoveAfterAction* _removeAfterAction;
		ScaleByPathAction* _scaleByPathAction;
		ColorFlashAction* _colorFlashAction;
		Actions m_Actions;
		BehaviorDefinitions* _behaviorDefinitions;

	};

}