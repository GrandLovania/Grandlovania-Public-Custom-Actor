
#include <z64ovl/oot/debug.h>     // game
#include <z64ovl/oot/helpers.h>

//HELPER_INCLUDE_helper_limb_focus( 33, 1300.0f, 800.0f, 1000.0f )
HELPER_INCLUDE_helper_limb_focus( 1, 0.0f, 0.0f, 0.0f )



#define OBJ_ID         0x78      // primary object ID the actor uses

#define ACT_ID         0x1A     // actor ID (optional), put 0xDEAD for zzromtool

#define PAD            0x0        // always 0, put 0xBEEF for zzromtool

#define MODEL          0x06001E30

#define COLLISION      0x060080E0

// Hierarchy
#define SKEL_GYORG     0x060093A8

// Animations
#define ANIM_SWIM     0x06009C14 // Swim
#define ANIM_DASH     0x0600A6C0 // Dash
#define ANIM_VULNERABLE     0x060099D0 // Vulnerable


// Animation IDs
#define ANIM_SWIM_ID     0 // Swim
#define ANIM_DASH_ID     1 // Dash
#define ANIM_VULNERABLE_ID     2 // Vulnerable




typedef struct {

	z64_actor_t actor;
	z64_skelanime_t skelanime;
	u16 flap, anim, speed;
	u16 angle;
	z64_actor_t Link;
	u8 cylinderreceptor[0x7C];
    u8 cylinderdamage[0x7C];
	vec3f_t initpos;
	u8 prevhealth;
	
	//Attack
	u16 attackchooser;
	u16 slashrad;
	
	//Necessesary
	u16 spidle;
	u16 spspinattack;
	u16 spminionattack;
	u16 spfallingrock;
	u16 spdeath;
	u16 spjump;
 

 	

} entity_t;

	

const u32 receptorhitbox[] =
{
0x07000939, 0x10010000, 0x00000000, 0x00000000, // 05 here
0x00000000, 0xFFCFFFFF, 0x00000000, 0x11050100, 
0x00560078, 0x00000000, 0x00000000 // first thing is hitbox itself
};

const u32 damaginghitbox[] =
{

0x0A110009, 0x20010000, 0x00000000, 0xFFCFFFFF, 
0x04101010, 0xFFCFFFFF, 0x00000000, 0x01000100, //Damage Amount 00 burn 01 freeze 02 shock 03 KB 04
0x00930080, 0x00000000, 0x00000000 // Damage Radius 0x00930080


};

const u8 damagechart[] =
{
0x10, 0xF1, 0x00, 0xF2, 0x10, 0xF2, 0x00, 0x00, 0xF1,
0xF1, 0xF1, 0x22, 0x32, 0x42, 0xF1, 0x10, 0x10, 0xF1,
0x10, 0x10, 0x00, 0x00, 0xF1, 0xE1, 0xD2, 0xD2, 0x00,
0x00, 0x00, 0x00, 0x0F, 0x02, 0x02, 0x02, 0x02, 0x02
};



const u32 unkchart[] = 
{
0x0100000F, 0x001E1E00
};


static void init(entity_t *en, z64_global_t *global) 
{
      
      // initialize skeleton for drawing, with animation
	_z_skelanime_mtx_init(global, &en->skelanime, SKEL_GYORG, ANIM_SWIM, 0, 0, 0);

          
	en->flap = 1;
	en->anim = ANIM_SWIM_ID;

      // scale actor's object model to 1/1000
	actor_set_scale(&en->actor, 0.2f);

      en->spidle = 0; //Idle
	  
	  en->spspinattack = 0; //Spin Attack
	  
	  en->spminionattack = 0; //Minion Attack
	  
	  en->spdeath = 0; //Death
	  
	  en->spjump = 0; //Jump

      u8 prevhealth = en->actor.health;
	  
	  
	  en->attackchooser = 0; //0 = Idle 1 = Spin Attack 2 = Minion Attack 3 = Slash at link 4 = Jump 5 = Flame 6 = Bat 7 = Jump 8 = Intro
	 
	   
	// en->actor.navi_msg = 0x012F;
	  
	  u32 temp = 0;
	  
	  en->angle =  0x0201;
	  
  
	  en->actor.target_dist_index = 9; //0 = short, 1 = medium, 2 = uber mega long, but short initial target distance, 3 = long, 4 = very long, 5 = very long?, 6 = very short, 7 = short, 8 = medium, 9 = infinite, 10 = can't target
	  
	  z64_actor_t Link = zh_get_player(global)->actor;
	  
	  z_collider_cylinder_alloc(global,&en->cylinderreceptor);
      z_collider_cylinder_init(global,&en->cylinderreceptor,&en->actor,&receptorhitbox);
      z_collider_cylinder_alloc(global,&en->cylinderdamage);
      z_collider_cylinder_init(global,&en->cylinderdamage,&en->actor,&damaginghitbox);

      external_func_80061ED4(AADDR(&en->actor,0x98),&damagechart,&unkchart); // damage chart
	
	  en->initpos = en->actor.pos;
	  
	  en->actor.health = 0x0040;

}

static void update(entity_t *en, z64_global_t *global) 
{
	
    zh_draw_debug_text(global, 0xFFFFFFFF, 5, 7, "Gyorg: %d", en->actor.health);

	
	u8 prevhealth = en->actor.health;
    actor_update_health(&en->actor);
        if (en->actor.health != prevhealth)
        {
          en->actor.damage_color = 0x405F;
        sound_play_actor( &en->actor, 0x3956);
        }



z64_actor_t Link = zh_get_player(global)->actor;



actor_collision_routine(global, &en->actor, 15.0f, 28.0f, 60.0f, 0x1D);



if (en->attackchooser == 0)
{
	//en->actor.pos.y = 0;
	u32 anim;
	//NECESSESARY
	en->spidle = 0;
	
	en->spminionattack = 0;
	en->spfallingrock += 1;
	//NECESSESARY

z_actor_move_dir_vel(&en->actor); //move in direction
actor_collision_routine(global, &en->actor, 15.0f, 28.0f, 60.0f, 0x1D);
if(en->spspinattack = 0)
{
	en->actor.dir.y += 200;
	en->actor.rot.y += 200;
}else{
	en->actor.dir.y += 450;
	en->actor.rot.y += 450;
} 
	en->actor.rot.x = 0;
	
    en->actor.xz_speed = 15.0f;
	
	if(165 > en->actor.pos.y)
	{
	 en->actor.pos.y += 3;	
	}
	if(165 < en->actor.pos.y)
	{
	 en->actor.pos.y -= 3;	
	}


if (en->spfallingrock == 1)
{
	 

anim = ANIM_SWIM;
en->anim = ANIM_SWIM_ID;
        z_skelanime_change_anim(
			&en->skelanime, anim, 1.0,
			0, 0, 0, 4
		);
}


if (en->spfallingrock == 120)
{
sound_play_actor( &en->actor, 0x3801);
en->spspinattack = 2;
}

if (en->spfallingrock == 160)
{
	en->attackchooser = 2;
	en->spidle = 0;
en->spspinattack = 1;
en->spminionattack = 0;
en->spfallingrock = 0;
en->spjump = 0;
}

if( Link.pos.y < 439)
{
	en->attackchooser = 1;
	en->spidle = 0;
en->spspinattack = 1;
en->spminionattack = 0;
en->spfallingrock = 0;
en->spjump = 0;
}

}

if (en->attackchooser == 1)
{
	//en->actor.pos.y = 0;
	u32 anim;
	//NECESSESARY
	en->spidle = 0;
	en->spspinattack = 1;
	en->spminionattack = 0;
	en->spfallingrock += 1;
	//NECESSESARY

z_actor_move_dir_vel(&en->actor); //move in direction
actor_collision_routine(global, &en->actor, 15.0f, 28.0f, 60.0f, 0x1D);
	en->actor.dir.y = en->actor.rot_toward_link_y;
	//en->actor.pos.y = Link.pos.y ;//* en->actor.xz_speed;
	en->actor.rot.y = en->actor.rot_toward_link_y;
	en->actor.rot.x = -Link.pos.y * 10;

	if(Link.pos.y > en->actor.pos.y)
	{
	 en->actor.pos.y += 8;	
	}
	if(Link.pos.y < en->actor.pos.y)
	{
	 en->actor.pos.y -= 8;	
	}

if (en->spfallingrock == 1)
{
	  en->actor.xz_speed = 25.0f;
sound_play_actor( &en->actor, 0x3802);
anim = ANIM_SWIM;
en->anim = ANIM_SWIM_ID;
        z_skelanime_change_anim(
			&en->skelanime, anim, 3.0,
			0, 0, 0, 4
		);
}

if (en->spfallingrock > 80)
{
	en->actor.xz_speed = 15.0f;
en->actor.rot.y = en->actor.dir.y;	 
en->actor.dir.y = -en->actor.rot_toward_link_y;
}

if (en->spfallingrock > 160)
{
en->actor.dir.y = en->actor.rot_toward_link_y;
	//en->actor.pos.y = Link.pos.y ;//* en->actor.xz_speed;
	en->actor.rot.y = en->actor.rot_toward_link_y;
    en->actor.xz_speed = 25.0f;
	en->spfallingrock = 0;
}

if( Link.pos.y = 439)
{
	en->attackchooser = 0;
	en->spidle = 0;
en->spspinattack = 1;
en->spminionattack = 0;
en->spfallingrock = 0;
en->spjump = 0;
}



}

if (en->attackchooser == 2) 
{
//en->actor.pos.y = 0;
	u32 anim;
	//NECESSESARY
	en->spidle = 0;
	
	en->spminionattack = 0;
	en->spfallingrock += 1;
	//NECESSESARY

z_actor_move_dir_vel(&en->actor); //move in direction
actor_collision_routine(global, &en->actor, 15.0f, 28.0f, 60.0f, 0x1D);
	
 
	if(Link.pos.y > en->actor.pos.y)
	{
	 en->actor.pos.y += 10;	
	}
	if(Link.pos.y < en->actor.pos.y)
	{
	 en->actor.pos.y -= 10;	
	}
	en->spspinattack = 1;


if (en->spfallingrock == 1)
{
	en->actor.dir.y = en->actor.rot_toward_link_y;
	//en->actor.pos.y = Link.pos.y ;//* en->actor.xz_speed;
	en->actor.rot.y = en->actor.rot_toward_link_y;
	  en->actor.xz_speed = 28.0f;

anim = ANIM_SWIM;
en->anim = ANIM_SWIM_ID;
        z_skelanime_change_anim(
			&en->skelanime, anim, 3.0,
			0, 0, 0, 4
		);
}

if (en->spfallingrock > 60)
{
	en->attackchooser = 0;
	en->spidle = 0;
en->spspinattack = 1;
en->spminionattack = 0;
en->spfallingrock = 1;
en->spjump = 0;
}


}


actor_collider_cylinder_update(&en->actor,&en->cylinderreceptor);
actor_collision_check_set_ot(global,AADDR(global,0x11E60),&en->cylinderreceptor);
actor_collision_check_set_ac(global,AADDR(global,0x11E60),&en->cylinderreceptor);

actor_collider_cylinder_update(&en->actor,&en->cylinderdamage);
actor_collision_check_set_at(global,AADDR(global,0x11E60),&en->cylinderdamage);
    


	
}

static void dest(entity_t *en, z64_global_t *global) 
{



}

static void draw(entity_t *en, z64_global_t *global) 
{
	z_skelanime_draw_table(&en->skelanime);
	
	_z_skelanime_draw_mtx(
			global
			, en->skelanime.limb_index
			, en->skelanime.draw_table_rot
			, 13
			, 0
			, &helper_limb_focus
			, &en->actor
		);
		
		f32 circleSize[] = {1.15f, 0.0f, 1.15f};
		
		actor_shadow_circle(&en->actor.pos, circleSize, 0xFF, global);
		
	
}

/* .data */
z64_actor_init_t init_vars = {
	.number = ACT_ID, 
	.type = 0x09, 
	.room = 0x00,
	.flags = 0x00000031,
	.object = OBJ_ID,
	.padding = PAD, 
	.instance_size = sizeof(entity_t),
	.init = init,
	.dest = dest,
	.main = update,
	.draw = draw
	
};

