#include "libs.h"
#include "Pi.h"
#include "Sfx.h"
#include "Frame.h"
#include "StarSystem.h"
#include "Space.h"
#include "Pi.h"
#include "TextureCache.h"
#include "render/Render.h"

static const int	s_maxSfxPerFrame	= 1024;
static const float	s_explosionLifespan	= 2.f;

Sfx::Sfx()
{
	m_type = TYPE_NONE;
}

void Sfx::Save(Serializer::Writer &wr)
{
	wr.Vector3d(m_pos);
	wr.Vector3d(m_vel);
	wr.Float(m_age);
	wr.Int32(m_type);
}

void Sfx::Load(Serializer::Reader &rd)
{
	m_pos = rd.Vector3d();
	m_vel = rd.Vector3d();
	m_age = rd.Float();
	m_type = static_cast<Sfx::TYPE>(rd.Int32());
}

void Sfx::Serialize(Serializer::Writer &wr, const Frame *f)
{
	// how many sfx turds are active in frame?
	int numActive = 0;
	if (f->m_sfx) {
		for (int i=0; i<s_maxSfxPerFrame; i++) {
			if (f->m_sfx[i].m_type != TYPE_NONE) numActive++;
		}
	}
	wr.Int32(numActive);

	if (numActive) for (int i=0; i<s_maxSfxPerFrame; i++) {
		if (f->m_sfx[i].m_type != TYPE_NONE) {
			f->m_sfx[i].Save(wr);
		}
	}
}

void Sfx::Unserialize(Serializer::Reader &rd, Frame *f)
{
	int numActive = rd.Int32();
	if (numActive) {
		f->m_sfx = new Sfx[s_maxSfxPerFrame];
		for (int i=0; i<numActive; i++) {
			f->m_sfx[i].Load(rd);
		}
	}
}

void Sfx::SetPosition(vector3d p)
{
	m_pos = p;
}

void Sfx::TimeStepUpdate(const float timeStep)
{
	m_age += timeStep;
	m_pos += m_vel * double(timeStep);

	switch (m_type) {
		case TYPE_EXPLOSION:
			if (m_age > s_explosionLifespan) m_type = TYPE_NONE;
			break;
		case TYPE_DAMAGE:
			if (m_age > 2.0) m_type = TYPE_NONE;
			break;
		case TYPE_NONE: break;
	}
}

void Sfx::Render(const matrix4x4d &ftransform)
{
	Texture *smokeTex = 0;
	Texture *sphereTex = 0;
	Texture *glowTex = 0;
	GLUquadricObj *sphere = 0;
	std::vector<Vertex> verts;
	float scale = 0;
	float fade = 0;
	float col[4];

	vector3d fpos = ftransform * GetPosition();

	switch (m_type) {
		case TYPE_NONE: break;
		case TYPE_EXPLOSION:
			verts.push_back(Vertex(vector3f(-1, -1, 0), 0.f, 1.f));
			verts.push_back(Vertex(vector3f(-1, 1, 0), 1.f, 1.f));
			verts.push_back(Vertex(vector3f(1, 1, 0), 1.f, 0.f));
			verts.push_back(Vertex(vector3f(1, -1, 0), 0.f, 0.f));

			Render::State::UseProgram(0);
			glDisable(GL_LIGHTING);
			glEnable(GL_TEXTURE_2D);
			glDepthMask(GL_FALSE);
			glDisable(GL_CULL_FACE);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glEnable(GL_BLEND);
			glPushMatrix();

			glTranslatef(fpos.x, fpos.y, fpos.z);
			fade = sqrt(m_age / s_explosionLifespan);
			scale = m_radius + (m_radius * 29 * fade);
			fade = 1 - fade;
			glScalef(scale, scale, scale);

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
			glPushMatrix();

			glowTex = Pi::textureCache->GetBillboardTexture(PIONEER_DATA_DIR "/textures/halo.png");
			glowTex->Bind();
			glColor4f(1,1,0.75,fade);
			glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &verts[0].pos);
			glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &verts[0].u);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

			glPopMatrix();
			glDisableClientState (GL_VERTEX_ARRAY);
			glDisableClientState (GL_TEXTURE_COORD_ARRAY);

			sphereTex = Pi::textureCache->GetBillboardTexture(PIONEER_DATA_DIR "/textures/explosion.png");
			sphereTex->Bind();
			sphere = gluNewQuadric();
			gluQuadricDrawStyle(sphere, GLU_FILL);
			gluQuadricNormals(sphere, GLU_SMOOTH);
			gluQuadricOrientation(sphere, GLU_OUTSIDE);
			gluQuadricTexture(sphere, GL_TRUE);
			glColor4f(1,0.2,0.1,fade);
			gluSphere(sphere, 1, 20,20);
			sphereTex->Unbind();

			glEnable(GL_CULL_FACE);
			glPopMatrix();
			break;
		case TYPE_DAMAGE:
			col[0] = 1.0f;
			col[1] = 1.0f;
			col[2] = 0.0f;
			col[3] = 1.0f-(m_age/2.0f);
			vector3f pos(&fpos.x);
			smokeTex = Pi::textureCache->GetBillboardTexture(PIONEER_DATA_DIR "/textures/smoke.png");
			smokeTex->Bind();
			Render::PutPointSprites(1, &pos, 20.0f, col);
			break;
	}
}

Sfx *Sfx::AllocSfxInFrame(Frame *f)
{
	if (!f->m_sfx) {
		f->m_sfx = new Sfx[s_maxSfxPerFrame];
	}

	for (int i=0; i<s_maxSfxPerFrame; i++) {
		if (f->m_sfx[i].m_type == TYPE_NONE) {
			return &f->m_sfx[i];
		}
	}
	return 0;
}

void Sfx::Add(const Body *b, TYPE t)
{
	Sfx *sfx = AllocSfxInFrame(b->GetFrame());
	if (!sfx) return;

	sfx->m_type = t;
	sfx->m_age = 0;
	sfx->SetPosition(b->GetPosition());
	sfx->m_vel = b->GetVelocity();
	switch (t) {
		case TYPE_DAMAGE:
			sfx->m_vel += 200.0 * vector3d(
				Pi::rng.Double() - 0.5,
				Pi::rng.Double() - 0.5,
				Pi::rng.Double() - 0.5);
			break;
		case TYPE_EXPLOSION:
			sfx->m_radius = b->GetBoundingRadius() / 2;
			break;
	}
}

void Sfx::TimeStepAll(const float timeStep, Frame *f)
{
	if (f->m_sfx) {
		for (int i=0; i<s_maxSfxPerFrame; i++) {
			if (f->m_sfx[i].m_type != TYPE_NONE) {
				f->m_sfx[i].TimeStepUpdate(timeStep);
			}
		}
	}
	
	for (std::list<Frame*>::iterator i = f->m_children.begin();
			i != f->m_children.end(); ++i) {
		TimeStepAll(timeStep, *i);
	}
}

void Sfx::RenderAll(const Frame *f, const Frame *camFrame)
{
	if (f->m_sfx) {
		matrix4x4d ftran;
		Frame::GetFrameTransform(f, camFrame, ftran);

		for (int i=0; i<s_maxSfxPerFrame; i++) {
			if (f->m_sfx[i].m_type != TYPE_NONE) {
				f->m_sfx[i].Render(ftran);
			}
		}
	}
	
	for (std::list<Frame*>::const_iterator i = f->m_children.begin();
			i != f->m_children.end(); ++i) {
		RenderAll(*i, camFrame);
	}
}
