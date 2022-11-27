#include "CharacterHair.h"

#include "..\..\..\Character.h"
#include "..\..\..\Components\CharacterPhysics.h"
/*
static const char *root_name = "hips_joint";
*/
CharacterHair::CharacterHair() :
	table(_FL_),
	ropes(_FL_),
	colls(_FL_)
{
	params.Reset();

	chr	  = null;
	anime = null;

	started = false;

//	root = 0;
	diff = 0.0f;

	empty = true;

	canUpdate = false;
}

void CharacterHair::Init(const InitData &data)
{
	anime  = data.animation;
	chr	   = data.chr;

	render = &chr->Render();
	params.Reset();

	FindParams();

	if( params.Ptr())
		Invalidate();
}

void CharacterHair::SetParams()
{
	HairParams *par = (HairParams *)params.Ptr();

	for( int i = 0 ; i < ropes ; i++ )
	{
		ropes[i].SetK(par->hair.lines[i].k);
		ropes[i].SetM(par->hair.lines[i].m);
		ropes[i].SetN(par->hair.lines[i].n);
	}
}

void CharacterHair::Draw(float dltTime)
{
	#ifndef _XBOX

	if( chr == null )
		return;

	bool edit_mode = chr->EditMode_IsOn();
	bool hair_debg = chr->arbiter->IsHairDebug();

	HairParams *p = (HairParams *)params.Ptr();

	if( edit_mode && (p && p->EditMode_IsSelect() || api->DebugKeyState('D')) ||
		hair_debg )
	{
	/*	Matrix m;

	//	chr->physics->GetModelMatrix(m);
		m = mat;*/

	/*	for( int j = 0 ; j < rope.sphes ; j++ )
		//	if( edit_mode )
		//		render->DrawSphere(m.MulVertex(rope.sphes[j].p),rope.sphes[j].r,rope.sphes[j].c);
		//	else
			//	render->DrawSphere(rope.sphes[j].p,rope.sphes[j].r,rope.sphes[j].c);
			{
				Vector p = rope.sphes[j].p;

				p = mat.MulNormalByInverse(p); p -= diff;
				p = currMat.MulVertex(p);

				render->DrawSphere(p,rope.sphes[j].r,rope.sphes[j].c);
			}*/

		Matrix a,b;

		for( int j = 0 ; j < colls ; j++ )
		{
			const Collider &c = colls[j];

			if( c.b < 0 )
			{
				a = anime->GetBoneMatrix(c.a);

				a *= currMat;

				render->DrawSphere(a.MulVertex(c.p),c.r,c.c);
			}
			else
			{
				a = anime->GetBoneMatrix(c.a);
				b = anime->GetBoneMatrix(c.b);

				a *= currMat;
				b *= currMat;

				Vector p = b.pos - a.pos;

				render->DrawSphere(a.MulVertex(c.p) + p*c.x,c.r,c.c);
			}
		}
	}

	if( edit_mode && api->DebugKeyState('S') ||
		hair_debg )
	{
		for( int i = 0 ; i < ropes ; i++ )
		{
			CharacterRope &rope = ropes[i];

			DrawRope(rope);
		}
	}

	#endif
}

void CharacterHair::Update(float dltTime)
{
	if( !canUpdate )
		return;

	if( empty || ropes < 1 )
		return;

	if( started )
	{
		if( chr && chr->physics )
		{
			chr->physics->GetModelMatrix(mat);// dltPos = mat.pos;

			currMat = mat;
			prevMat = mat;

			started = false;
		}
	}

/*	if( chr && chr->physics )
	{
		Matrix m; chr->physics->GetModelMatrix(m);
		api->Trace("    MATRIX %s: %f %f %f",anime->GetName(),m.pos.x,m.pos.y,m.pos.z);
	}*/

	bool edit_mode = chr->EditMode_IsOn();

	if( edit_mode )
	{		
		FindParams();

		HairParams *p = (HairParams *)params.Ptr();

		if( p && p->updated || !p && !empty )
			Invalidate();
	}

//	if( params )
//		SetParams();

	float tk = 1.0f;

	if( dltTime > 0.02f )
		tk = 0.8f;
/*
//	if( dltTime > 0.02f )
//		dltTime = 0.02f;
	if( dltTime > 0.03f )
		dltTime = 0.03f;*/

//	if( dltTime < 0.01f )
//		dltTime = 0.01f;
	if( dltTime < 0.005f )
		dltTime = 0.005f;

//	chr->physics->GetModelMatrix(mat);

/*	float dd = dltTime*50.0f;

	if( dd < 0.1f )
		dd = 0.1f;

	if( dd > 0.5f )
		dd = 0.5f;

	mat.pos.Lerp(dltPos,mat.pos,dd);*/

//	mat.pos.Lerp(dltPos,mat.pos,0.5f);
//	mat.pos.Lerp(dltPos,mat.pos,0.3f);

/*	float yy = mat.pos.y;

	mat.pos.Lerp(dltPos,mat.pos,0.4f); mat.pos.y = yy;

	dltPos = mat.pos;*/

/*	if( started )
	{
		if( chr && chr->physics )
		{
			chr->physics->GetModelMatrix(mat);

			currMat = mat;
			prevMat = mat;
		}

		apos = anime->GetBonePosition(root);
		prep = apos;

		diff = 0.0f;

		started = false;
	}
	else*/
//	{
		Vector q = currMat.pos - prevMat.pos;

		q.x *= 0.3f*tk;
		q.z *= 0.3f*tk;
		q.y *= 0.3f*tk;

		if( q.x*q.x + q.y*q.y + q.z*q.z > 1.0f )
			q = 0.0f;

		Vector p = mat.pos + q;

		mat = currMat;
		mat.pos = p;

		prevMat = currMat;

	//	if( root >= 0 )
	//	{
			curp = anime->GetBonePosition(/*root*/0);

			q = curp - prep;

			q.x *= 0.3f;
			q.z *= 0.3f;
		//	q.y *= 0.3f;

			p = apos + q;

			apos = p;

			prep = curp; diff = apos - curp;
	//	}
//	}

	float ty = 0.0f;

	for( int i = 0 ; i < ropes ; i++ )
	{
		CharacterRope &rope = ropes[i];

		rope.SetGlobal(mat);

		Matrix a = anime->GetBoneMatrix(rope.m_a);
		Matrix b = anime->GetBoneMatrix(rope.m_b);

		a.pos += diff;
		b.pos += diff;

	/*	const float bk = 0.5f;

		float ay = a.pos.y;
		float by = b.pos.y;

		a.pos.Lerp(rope._a,a.pos,bk); a.pos.y = ay; rope._a = a.pos;
		b.pos.Lerp(rope._b,b.pos,bk); b.pos.y = by; rope._b = b.pos;*/

	//	mat.pos += dltPos*dltTime*0.9f;

		a *= mat;
		b *= mat;

		Vector org = b.pos;
		Vector dir = b.pos - a.pos;

		dir.Normalize();

		rope.SetOrigin(org);
		rope.SetDir	  (dir);

		for( int j = 0 ; j < colls ; j++ )
		{
			const Collider &c = colls[j];

			if( c.b < 0 )
			{
				a = anime->GetBoneMatrix(c.a); a.pos += diff;

				a *= mat;

				rope.SetSphere(j,a.MulVertex(c.p),c.r,c.c);
			}
			else
			{
				a = anime->GetBoneMatrix(c.a); a.pos += diff;
				b = anime->GetBoneMatrix(c.b); b.pos += diff;

				a *= mat;
				b *= mat;

				Vector p = b.pos - a.pos;

				rope.SetSphere(j,a.MulVertex(c.p) + p*c.x,c.r,c.c);
			}
		}

		//
		// DrawShperes -> Draw()
		//

		rope.Beg(dltTime);
	}

	for( int i = 0 ; i < ropes ; i++ )
		for( int j = 0 ; j < ropes ; j++ )
			if( i != j )
				ropes[i].Collide(ropes[j]);

	for( int i = 0 ; i < ropes ; i++ )
	{
		CharacterRope &rope = ropes[i];

		rope.End(dltTime);

	/*	#ifndef _XBOX

		if( edit_mode && GetAsyncKeyState('S') < 0 ||
			chr && chr->arbiter->IsHairDebug())
			DrawRope(rope);

		#endif*/
	//
	//	DrawRopes -> Draw()
	//

	/*	for( int j = -1 ; j < rope - 1 ; j++ )
		{
			{
				Matrix m = rope.GetMatrix(j + 1);

				m *= currMat;

				m.pos = currMat.MulVertex(mat.MulVertexByInverse(j < 0 ? rope.GetOrigin() : rope[j]) - diff);

				render->DrawMatrix(m,0.02f);
			}

			{
				Matrix m = rope.GetMatrix(j);

				m *= currMat;

				m.pos = currMat.MulVertex(mat.MulVertexByInverse(j < 0 ? rope.GetOrigin() : rope[j]) - diff);

				Vector n = m.MulNormal(rope.GetNormal(j));

				n *= 0.05;
				n += m.pos;

				render->DrawVector(m.pos,n,0xffffff00);
			}
		}*/
	}
}

void CharacterHair::GetBoneTransform(int i, BlendData &data)
{
	canUpdate = true;

	Place		   &place = table[i];
	CharacterRope  &rope  = ropes[place.i];

	Assert(rope > 0)

	if( place.j >= rope )
	{
		data.position = data.prevPosition;
		data.rotation = data.prevRotation;
		data.scale	  = data.prevScale;

	/*	api->Trace("\n----------------------\n");
		api->Trace("HAIR BUG: character %s, animation %s, rope %d, node %d",
			chr ? chr->GetObjectID() : "n/a",
			anime ? anime->GetName() : "n/a",place.i,place.j);
		api->Trace("\n----------------------\n");*/

		return;
	}

	Vector p = place.j < 0 ? rope.GetOrigin() : rope[place.j];
	Vector n = place.j + 1 < rope ? rope[place.j + 1] : rope[place.j];

	Matrix m = anime->GetBoneMatrix(anime->GetBoneParent(data.boneIndex));

//	if( place.j > 0 )
		rope.SetMatrix(place.j,m);

/*	m = m*mat;

	p = m.MulVertexByInverse(p);
	n = m.MulVertexByInverse(n);*/

	p = mat.MulVertexByInverse(p); p -= diff;
	n = mat.MulVertexByInverse(n); n -= diff;

	p = m.MulVertexByInverse(p);
	n = m.MulVertexByInverse(n);

	m = m*mat;

	if( place.j < 0 )
	{
		Vector t = data.prevPosition - p;

		p  = data.prevPosition;
		n += t;
	}
	else
	{
		Vector q = m.MulVertexByInverse(rope.GetBonePos(place.j));
		Vector t = q - p;

		p  = q;
		n += t;
	}

	if( place.j + 1 < rope )
		rope.SetBonePos(place.j + 1,m.MulVertex(n));

	Matrix anim;

	data.prevRotation.GetMatrix(anim);

	Matrix z;
	Matrix r;

/*	Vector up;

	switch( rope.up )
	{
		case 0:
			up = anim.vx;
			break;
		case 1:
			up = anim.vy;
			break;
		case 2:
			up = anim.vz;
			break;
	}*/

	switch( rope.m_t )
	{
		case 0: // -x
		//	z.BuildView(p,n,rope.up < 0 ? anim.vy : up);
			z.BuildView(p,n,anim.vy);
			r.BuildRotateY(-PI*0.5f);
			rope.SetNormal(place.j,-anim.vx);
			break;

		case 3: // +x
		//	z.BuildView(p,n,rope.up < 0 ? anim.vy : up);
			z.BuildView(p,n,anim.vy);
			r.BuildRotateY( PI*0.5f);
			rope.SetNormal(place.j, anim.vx);
			break;

		case 1: // -y
		//	z.BuildView(p,n,rope.up < 0 ? anim.vz : up);
			z.BuildView(p,n,anim.vz);
			r.BuildRotateX( PI*0.5f);
			rope.SetNormal(place.j,-anim.vy);
			break;

		case 4: // +y
		//	z.BuildView(p,n,rope.up < 0 ? anim.vz : up);
			z.BuildView(p,n,anim.vz);
			r.BuildRotateX(-PI*0.5f);
			rope.SetNormal(place.j, anim.vy);
			break;

		case 2: // +z
		//	z.BuildView(p,n,rope.up < 0 ? anim.vy : up);
			z.BuildView(p,n,anim.vy);
			;
			rope.SetNormal(place.j, anim.vz);
			break;

		default:;
		//	api->Trace("CharacterHair::GetBoneTransform(): invalid bone orientation");
		//	Assert(0)
	}

	z = z*r; z.Inverse();

//	z = anim;

	data.rotation.Set(z);

/*	if( _isnan(data.rotation.x) ||
		_isnan(data.rotation.y) ||
		_isnan(data.rotation.z) ||
		_isnan(data.rotation.w))
		Assert(0)*/

//	if( place.j < 0 )
//		p = data.prevPosition;

	data.position = p;
//	data.position = data.prevPosition;

	data.scale	  = 1.0f;

/*	Matrix mm;

	chr->physics->GetModelMatrix(mm);

	z *= anime->GetBoneMatrix(anime->GetBoneParent(data.boneIndex));
	z *= mm;

	render->DrawMatrix(z,0.02f);

	chr->physics->GetModelMatrix(mm);

	z = anim;

	z *= anime->GetBoneMatrix(anime->GetBoneParent(data.boneIndex));
	z *= mm;

	render->DrawMatrix(z,0.05f);*/
}

void CharacterHair::Invalidate()
{
	canUpdate = false;

/*	api->Trace(anime->GetName());

	long nbones = anime->GetNumBones();

	for( int i = 0 ; i < nbones ; i++ )
	{
		api->Trace("%3d %s",i,anime->GetBoneName(i));
	}*/

	ReleaseBones();

	table.DelAll();

	ropes.DelAll();
	colls.DelAll();

	empty = true;

	HairParams *par = (HairParams *)params.Ptr();

	if( !par )
		return;

	empty = false;

	par->updated = false;

/*	if( chr && chr->physics )
	{
		chr->physics->GetModelMatrix(mat);// dltPos = mat.pos;

		currMat = mat;
		prevMat = mat;
	}*/

	started = true;

	Disable();

	const HairParams::HairData &hair = par->hair;
	const HairParams::CollList &coll = par->coll;

	for( int i = 0 ; i < coll.list ; i++ )
	{
		const HairParams::CollData &data = coll.list[i];

		int ai = anime->FindBone(data.a.c_str(),true);
		int bi = anime->FindBone(data.b.c_str(),true);

		if( ai < 0 )
			continue;

		Collider &c = colls[colls.Add()];

		c.a = ai;
		c.b = bi;

		c.x = data.x;
		c.r = data.r;

		c.p = data.p;

		c.c = data.c;
	}

	int index = -1; int rope_i = 0;

	for( int i = 0 ; i < hair.lines ; i++ )
	{
		const HairParams::HairLine &line = hair.lines[i];

	//	if( line.bones < 3 || line.n < line.bones )
	//		continue;

		int ai = anime->FindBone(line.bones[0].c_str(),true);
		int bi = anime->FindBone(line.bones[1].c_str(),true);

		if( ai < 0 || bi < 0 || anime->FindBone(line.bones[2].c_str(),true) < 0 )
			continue;

		CharacterRope &rope = ropes[ropes.Add()];

	//	rope.up = line.up;

		rope.m_a = ai;
		rope.m_b = bi;

		index = bi;

	//	int r_index = index;
	//	int r_table = table;

		RegBone(index,table);
		table.Add(Place(rope_i,-1));

	//	Assert(anime->GetBoneParent(bi) == ai)

		Matrix a = anime->GetBoneMatrix(rope.m_a);
		Matrix b = anime->GetBoneMatrix(rope.m_b);

	//	root = anime->FindBone(root_name,true);

	//	if( root >= 0 )
	//	{
			apos = anime->GetBonePosition(/*root*/0);
			prep = apos;
	//	}

		diff = 0.0f;

		a *= mat;
		b *= mat;

		Vector org = b.pos;
		Vector dir = b.pos - a.pos;

		dir.Normalize();

		rope.Create(org,dir);

		rope.m_t = -1;

		int prev = index;

		for( int j = 2 ;; j++ )
		{
			if( j < line.bones )
				index = anime->FindBone(line.bones[j].c_str(),true);
			else
				index++;

			////

		/*	if( index >= anime->GetNumBones())
				break;

			if( anime->GetBoneParent(index) != prev )
				break;*/

			if( index < 0 || index >= anime->GetNumBones() ||
				anime->GetBoneParent(index) != prev )
			{
				long n = anime->GetNumBones();

				for( index = 0 ; index < n ; index++ )
				{
					if( anime->GetBoneParent(index) == prev )
						break;
				}
				
				if( index >= n )
					break;
			}

			////

		//	Assert(anime->GetBoneParent(index) == prev)

			Matrix ma = anime->GetBoneMatrix(prev);
			Matrix mb = anime->GetBoneMatrix(index);

		//	;
			float len = (mb.pos - ma.pos).GetLength();

		//	api->Trace("INV LINE %d - %d: %f",prev,index,len);

			if( rope.m_t < 0 )
			{
				Vector q = mb.pos - ma.pos;
				
				q = ma.MulNormalByInverse(q); q.Normalize();

			//	api->Trace("%f,%f,%f",q.x,q.y,q.z);

				if( fabsf(q.x) > 0.9f )
				{
					if( q.x > 0.9f )
						rope.m_t = 3;
					else
						rope.m_t = 0;
				}
				if( fabsf(q.y) > 0.9f )
				{
					if( q.y > 0.9f )
						rope.m_t = 4;
					else
						rope.m_t = 1;
				}
				if( fabsf(q.z) > 0.9f )
					rope.m_t = 2;
			}

		//	api->Trace("%d",rope_i);

			ma *= mat;
			mb *= mat;

		//	float len = (mb.pos - ma.pos).GetLength();

		//	rope.Add(len);
			rope.Add(len,mb.pos);

			RegBone(index,table); table.Add(Place(rope_i,j - 2));

			rope.AddSphere(colls);

			prev = index;
		}

	//	RegBone(r_index,r_table);

		rope_i++;
	}

	SetParams();

//	dltPos = 0.0f;

/*	if( chr && chr->physics )
	{
		Matrix m; chr->physics->GetModelMatrix(m); dltPos = m.pos;
	}*/

	Enable();
}

void CharacterHair::ResetAnimation(IAnimation *animation)
{
	canUpdate = false;

	if( !animation ) // в принципе не нужно
		return;

	anime = animation;

//	ReleaseBones();

	table.DelAll();

//	ropes.DelAll();
	colls.DelAll();

	bool edit_mode = chr->EditMode_IsOn();

	if( edit_mode )
	{
		// ENgine: Hard Hack!!!
		params.Reset();

		FindParams();
	}

	HairParams *par = (HairParams *)params.Ptr();

	if( !par )
		return;

	par->updated = false;

/*	if( chr && chr->physics )
	{
		chr->physics->GetModelMatrix(mat);// dltPos = mat.pos;

		currMat = mat;
		prevMat = mat;
	}

	started = true;*/

	Disable();

	const HairParams::HairData &hair = par->hair;
	const HairParams::CollList &coll = par->coll;

	for( int i = 0 ; i < coll.list ; i++ )
	{
		const HairParams::CollData &data = coll.list[i];

		int ai = anime->FindBone(data.a.c_str(),true);
		int bi = anime->FindBone(data.b.c_str(),true);

		if( ai < 0 )
			continue;

		Collider &c = colls[colls.Add()];

		c.a = ai;
		c.b = bi;

		c.x = data.x;
		c.r = data.r;

		c.p = data.p;

		c.c = data.c;
	}

	int index = -1; // индекс текущей кости
	int rope_i = 0; // индекс текущей веревки (валидной цепочки)

	for( int i = 0 ; i < hair.lines ; i++ )
	{
		const HairParams::HairLine &line = hair.lines[i];

	//	if( line.bones < 3 || line.n < line.bones )
	//		continue;

		if( line.bones < 3 ) // в принципе не нужно
			continue;

		int ai = anime->FindBone(line.bones[0].c_str(),true);
		int bi = anime->FindBone(line.bones[1].c_str(),true);

		if( ai < 0 || bi < 0 || anime->FindBone(line.bones[2].c_str(),true) < 0 )
			continue;

		bool new_rope = ropes <= i;

		CharacterRope &rope = new_rope ? ropes[ropes.Add()] : ropes[i];

	//	rope.up = line.up;

		rope.m_a = ai;
		rope.m_b = bi;

		index = bi;

	//	int r_index = index;
	//	int r_table = table;

		RegBone(index,table);
		table.Add(Place(rope_i,-1));

	//	Assert(anime->GetBoneParent(bi) == ai)

		Matrix a = anime->GetBoneMatrix(rope.m_a);
		Matrix b = anime->GetBoneMatrix(rope.m_b);

		a.pos += diff;
		b.pos += diff;

	/*	root = anime->FindBone(root_name,true);

		if( root >= 0 )
		{
			apos = anime->GetBonePosition(root);
			prep = apos;
		}

		diff = 0.0f;*/

		a *= mat;
		b *= mat;

		Vector org = b.pos;
		Vector dir = b.pos - a.pos;

		dir.Normalize();

		rope.Create(org,dir,new_rope);

		if( new_rope )
			rope.m_t = -1;

		int prev = index;  // индекс предыдущей кости

		for( int j = 2 ;; j++ )
		{
			int k = j - 2; // индекс сегмента

			if( j < line.bones )
				// берем кость из таблицы
				index = anime->FindBone(line.bones[j].c_str(),true);
			else
				// берем следующую кость
				index++;

			////

		/*	if( index >= anime->GetNumBones())
				break;

			if( anime->GetBoneParent(index) != prev ) // перешли на следующую цепочку
				break;*/

			if( index < 0 || index >= anime->GetNumBones() ||
				anime->GetBoneParent(index) != prev )
			{
				long n = anime->GetNumBones();

				for( index = 0 ; index < n ; index++ )
				{
					if( anime->GetBoneParent(index) == prev )
						break;
				}
				
				if( index >= n )
					break;
			}

			////

		//	Assert(anime->GetBoneParent(index) == prev)

			Matrix ma = anime->GetBoneMatrix(prev);
			Matrix mb = anime->GetBoneMatrix(index);

			if( rope.m_t < 0 )
			{
				Vector q = mb.pos - ma.pos;
				
				q = ma.MulNormalByInverse(q); q.Normalize();

			//	api->Trace("%f,%f,%f",q.x,q.y,q.z);

				if( fabsf(q.x) > 0.9f )
				{
					if( q.x > 0.9f )
						rope.m_t = 3;
					else
						rope.m_t = 0;
				}
				if( fabsf(q.y) > 0.9f )
				{
					if( q.y > 0.9f )
						rope.m_t = 4;
					else
						rope.m_t = 1;
				}
				if( fabsf(q.z) > 0.9f )
					rope.m_t = 2;
			}

		//	;
			float len = (mb.pos - ma.pos).GetLength();

		//	api->Trace("RES LINE %d - %d: %f",prev,index,len);
			
		/*	if( len > 0.5f )
			{
				const char *bna = anime->GetBoneName(prev);
				const char *bnb = anime->GetBoneName(index);

				len += 0.0f;
			}*/

			ma.pos += diff;
			mb.pos += diff;

			ma *= mat;
			mb *= mat;

		//	float len = (mb.pos - ma.pos).GetLength();

			if( rope > k )
			{
			//	rope.SetSegmentLen(k,len); // обновить длину сегмента
			//	rope.SetSegmentLen(k,len,mb.pos);
				float old = rope.GetSegLen(k);
				if( len > old )
					len = old;
				rope.SetSegmentLen(k,len,mb.pos);
			}
			else
			//	rope.Add(len);
				rope.Add(len,mb.pos);

		//	api->Trace("RES LINE %d - %d: %f",prev,index,len);

			RegBone(index,table); table.Add(Place(rope_i,k));

		//	;
			rope.sphes.DelAll();

		//	if( new_rope )
				rope.AddSphere(colls);

			prev = index;
		}

	//	RegBone(r_index,r_table);

		rope_i++;
	}

	if( ropes > rope_i )
		ropes.DelRange(rope_i,ropes - 1);

	SetParams();

//	dltPos = 0.0f;

/*	if( chr && chr->physics )
	{
		Matrix m; chr->physics->GetModelMatrix(m); dltPos = m.pos;
	}*/

	Enable();
}

void CharacterHair::SetMatrix(const Matrix &m)
{
//	dltPos = mat.pos;

//	mat = m;
	currMat = m;

/*	if( started )
	{
	//	if( chr && chr->physics )
		{
	//		chr->physics->GetModelMatrix(mat);// dltPos = mat.pos;
			mat = m;

		//	currMat = mat;
			prevMat = mat;
		}

	//	root = anime->FindBone(root_name,true);

	//	if( root >= 0 )
	//	{
			apos = anime->GetBonePosition(root);
			prep = apos;
	//	}

		diff = 0.0f;

		started = false;
	}*/

//	mat.pos.Lerp(dltPos,mat.pos,0.01f); dltPos = mat.pos;

//	mat.pos = 0.0f;
}

void CharacterHair::DrawRope(CharacterRope &rope)
{
//	bool editMode = chr->EditMode_IsOn();

/*	Matrix m;

	chr->physics->GetModelMatrix(m);*/

	Vector p;
//	Vector q = m.MulVertex(rope.GetOrigin());
	Vector q = rope.GetOrigin();

	q = mat.MulVertexByInverse(q); q -= diff;
	q = currMat.MulVertex(q);

//	if( editMode )
//		q = m.MulVertex(q);

//	Vector f;

	const float r = 0.005f;

	render->DrawSphere(q,r,0xffff0000);

	for( int i = 0 ; i < rope ; i++, q = p )
	{
	//	p = m.MulVertex(rope[i]);
		p = rope[i];

		p = mat.MulVertexByInverse(p); p -= diff;
		p = currMat.MulVertex(p);

	//	if( editMode )
	//		p = m.MulVertex(p);

		const dword c = 0xff0000ff;
		const dword d = 0xffff0000;

	//	f = rope.GetForce(i);

		Matrix n;
			   n.BuildOriented(q,p,Vector(0.0f,1.0f,0.0f));

		render->DrawSolidBox(Vector(-r,-r,0.0f),Vector(r,r,rope.GetSegLen(i)),n,c);

	/*	Matrix m = rope.getMatrix(i);

		m.pos = p;

		float len = rope.GetSegLen(i);

		Vector a,b;

		switch( rope.m_t )
		{
			case 0: // +x
				a.Set(0.0f,-r,-r);
				b.Set( len, r, r);
				break;

			case 3: // -x
				a.Set(0.0f,-r,-r);
				b.Set(-len, r, r);
				break;

			case 1: // +y
				a.Set(-r,0.0f,-r);
				b.Set( r, len, r);
				break;

			case 4: // -y
				a.Set(-r,0.0f,-r);
				b.Set( r,-len, r);
				break;

			case 2: // +z
				a.Set(-r,-r,0.0f);
				b.Set( r, r, len);
				break;

			default:;
		}

		render->DrawSolidBox(a,b,m,c);*/
	}
}

void CharacterHair::FindParams()
{
	if( !params.Validate())
	{
		if( chr->pattern )
		{
			if( chr->FindObject(chr->pattern->cHairParams,params))
			{
				MO_IS_IF_NOT(id_HairParams, "HairParams", params.Ptr())
				{
					params.Reset();
				}
			}
		}		
	}
}
