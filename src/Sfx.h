#ifndef _SFX_H
#define _SFX_H

#include "Body.h"
#include "Serializer.h"

class Frame;

class Sfx {
public:
	enum TYPE { TYPE_NONE, TYPE_EXPLOSION, TYPE_DAMAGE };

	static void Add(const Body *, TYPE);
	static void TimeStepAll(const float timeStep, Frame *f);
	static void RenderAll(const Frame *f, const Frame *camFrame);
	static void Serialize(Serializer::Writer &wr, const Frame *f);
	static void Unserialize(Serializer::Reader &rd, Frame *f);

	Sfx();
	void SetPosition(vector3d p);
	vector3d GetPosition() const { return m_pos; }
private:
	static Sfx *AllocSfxInFrame(Frame *f);

	void Render(const matrix4x4d &transform);
	void TimeStepUpdate(const float timeStep);
	void Save(Serializer::Writer &wr);
	void Load(Serializer::Reader &rd);

	vector3d m_pos;
	vector3d m_vel;
	float m_age;
	float m_radius;
	enum TYPE m_type;

	struct Vertex {
		vector3f pos;
		float u;
		float v;
		Vertex(const vector3f &_pos, float _u, float _v) :
			pos(_pos), u(_u), v(_v) { }
	};
};

#endif /* _SFX_H */
