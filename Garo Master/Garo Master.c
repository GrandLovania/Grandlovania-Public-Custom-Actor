
#include <z64ovl/oot/debug.h>     // game
#include <z64ovl/oot/helpers.h>

//HELPER_INCLUDE_helper_limb_focus( 33, 1300.0f, 800.0f, 1000.0f )
HELPER_INCLUDE_helper_limb_focus( 11, 0.0f, 0.0f, 0.0f )



#define OBJ_ID         0x3A       // primary object ID the actor uses

#define ACT_ID         0x17       // actor ID (optional), put 0xDEAD for zzromtool

#define PAD            0x0        // always 0, put 0xBEEF for zzromtool

#define MODEL          0x06001E30

#define COLLISION      0x060010E0

// Hierarchy
#define SKEL_BOB     0x06003168

// Animations
#define ANIM_SIMPLESLASH     0x06003530 // Simple Slash
#define ANIM_SIMPLESLASHEND     0x0600378C // Simple Slash End
#define ANIM_SIMPLESLASHIDLE     0x060037AC // Simple Slash Idle
#define ANIM_LAND     0x06004384 // Land after epicness
#define ANIM_DIE     0x060044F8 // Die
#define ANIM_EPICINTRO    0x06005778 // Epic intro
#define ANIM_DIE_CROUCH    0x060063A4 // Death Crouch
#define ANIM_DTC    0x060067F0 // Die to Death Crouch
#define ANIM_IDLE    0x0600AE80 // Idle
#define ANIM_WALK    0x0600B5F0 // Walk/Jump
#define ANIM_SPIN   0x060072AC // Attack Spin
#define ANIM_WAR   0x0600788C // Where are you?



// Animation IDs
#define ANIM_SIMPLESLASH_ID     0 // Simple Slash
#define ANIM_SIMPLESLASHEND_ID     1 // Simple Slash End
#define ANIM_SIMPLESLASHENDIDLE_ID     2 // Simple Slash Ile
#define ANIM_LAND_ID     3 // Land after epicness
#define ANIM_DIE_ID     4 // Die
#define ANIM_EPICINTRO_ID     5 // Epic intro
#define ANIM_DIE_CROUCH_ID     6 // Death Crouch
#define ANIM_DTC_ID     7 // Die to Death Crouch
#define ANIM_IDLE_ID     8 // Idle
#define ANIM_WALK_ID     9 // Walk
#define ANIM_SPIN_ID     10 // Attack Spin
#define ANIM_WAR_ID     11 // Where are you?




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


	//Necessesary
	u16 timerone;
	u16 timertwo;
	u16 timerdie;
	u16 attackchooser;
	
	//z64_actor_t *explosionInstance = actor_spawn(AADDR(global, 0x1C24), global, 0x0010, en->actor.pos.x, en->actor.pos.y, en->actor.pos.z, 0, 0, 0, 0x0000);

 

 	

} entity_t;

	

const u32 receptorhitbox[] =
{
0x07000939, 0x10010000, 0x00000000, 0x00000000, // 05 here
0x00000000, 0xFFCFFFFF, 0x00000000, 0x11050100, 
0x00400040, 0x00000000, 0x00000000 // first thing is hitbox itself
};

const u32 damaginghitbox[] =
{

0x0A110009, 0x20010000, 0x00000000, 0xFFCFFFFF, 
0x01080400, 0xFFCFFFFF, 0x00000000, 0x01000100, //Damage Amount 00 burn 01 freeze 02 shock 03 KB 04
0x00400040, 0x00000000, 0x00000000 // Damage Radius 0x00930080


};


const u8 damagechart[] =
{
0x00, 0x00, 0x00, 0x04, 0x01, 0x02, 0x04, 0x00, 
0x02, 0x02, 0x02, 0x04, 0x03, 0x5F, 0x02, 0x02, 
0x02, 0x0F, 0x02, 0x02, 0x03, 0x02, 0x02, 0x02, 
0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02


};

const u32 unkchart[] = 
{
0x0100000F, 0x001E1E00
};


static void init(entity_t *en, z64_global_t *global) 
{
      
      // initialize skeleton for drawing, with animation
	  _z_skelanime_mtx_init(global, &en->skelanime, SKEL_BOB, ANIM_IDLE, 0, 0, 0);

      en->anim = ANIM_IDLE_ID;

      // scale actor's object model to 1/1000
	  actor_set_scale(&en->actor, 0.04f);

      en->timerone = 0;
	  en->timertwo = 0;
	  en->timerdie = 0;

      u8 prevhealth = en->actor.health;
	   
	  en->attackchooser = 69; //0 = Idle 1 = Go Toward Link 2 = Spin like a moron 69 = Spawn
	  sound_set_bgm(0x00);
	  
	  en->actor.pos.y = -100;

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
	  
	  en->actor.health = 0x0042;

}

static void update(entity_t *en, z64_global_t *global) 
{
	
	u32 anim;
	z64_actor_t Link = zh_get_player(global)->actor;
	
	
if (en->attackchooser == 69) //Before Fight
{
	u32 anim;
	en->timerone += 1;

       z_actor_move_dir_vel(&en->actor); //move in direction
       actor_collision_routine(global, &en->actor, 15.0f, 28.0f, 60.0f, 0x1D);
       en->actor.xz_speed = 0.0f;
       if (en->timerone == 1)
       {
                  anim = ANIM_IDLE;
                  en->anim = ANIM_IDLE_ID;
                    z_skelanime_change_anim(
			        &en->skelanime, anim, 1.0,
			           0, 0, 0
		         );
        }
		
		if (en->actor.dist_from_link_xz < 180.0f)
        {
            en->attackchooser = 70;
            en->timerone = 0;
            en->timertwo = 0;
        }
}

if (en->attackchooser == 70) //Jump to start fight
{
	u32 anim;
	en->timerone += 1;
	


       z_actor_move_dir_vel(&en->actor); //move in direction
       actor_collision_routine(global, &en->actor, 15.0f, 28.0f, 60.0f, 0x1D);
       en->actor.xz_speed = 0.0f;
       if (en->timerone == 1)
       {
		      
		   
                  anim = ANIM_EPICINTRO;
                  en->anim = ANIM_EPICINTRO_ID;
                    z_skelanime_change_anim(
			        &en->skelanime, anim, 1.0,
			           0, 0, 0
		         );
       } 
	   
	   if (en->timerone > 1)
       {
		 en->actor.pos.y = 35;
       }
	   
	   if (en->timerone == 20)
       {
		        sound_play_actor( &en->actor, 0x3844);
       }
	   
	   if (en->timerone == 43)
       {
		        sound_play_actor( &en->actor, 0x1801);
       }
	   
	   if (en->timerone == 50)
       {
		        sound_set_bgm(0x38);
				en->attackchooser = 1;
                en->timerone = 0;
                en->timertwo = 0;
       }
}

if (en->attackchooser == 1) //Walk
{
	u32 anim;
	en->timerone += 1;
    en->timertwo += 1;

       z_actor_move_dir_vel(&en->actor); //move in direction
       actor_collision_routine(global, &en->actor, 15.0f, 28.0f, 60.0f, 0x1D);
       en->actor.xz_speed = 2.89f;
	   en->actor.dir.y = en->actor.rot_toward_link_y;
       en->actor.rot.y = en->actor.rot_toward_link_y;
	   
       if (en->timerone == 1)
       {
		   en->actor.pos.y = 35;
                  anim = ANIM_WALK;
                  en->anim = ANIM_WALK_ID;
                    z_skelanime_change_anim(
			        &en->skelanime, anim, 1.0,
			           0, 0, 0
		         );
       }
	   
	   if(en->actor.pos.y < 35)
	   {
		   en->actor.pos.y = 35;
	   }
	   
	   //Jump
	   if (en->timerone == 0)
       {
		    en->actor.pos.y = 35;
	   }
	   
	   if (en->timerone == 1)
       {
		   en->actor.pos.y = 35;
	   }
		   
	   if (en->timerone == 2)
       {
		   en->actor.pos.y = 35;
	   }
	   
	   if (en->timerone == 3)
       {
		   en->actor.pos.y = 45;
	   }
	   
	   if (en->timerone == 4)
       {
		   en->actor.pos.y = 55;
	   }
	   
	   if (en->timerone == 5)
       {
		   en->actor.pos.y = 65;
	   }
	   
	   if (en->timerone == 6)
       {
		   en->actor.pos.y = 75;
	   }
	   if (en->timerone == 7)
       {
		   en->actor.pos.y = 85;
	   }
	   
	   //Fall
	   if (en->timerone == 8)
       {
		   en->actor.pos.y = 85;
	   }
	   if (en->timerone == 9)
       {
		   en->actor.pos.y = 75;
	   }
	   if (en->timerone == 10)
       {
		   en->actor.pos.y = 65;
	   }	   
	   if (en->timerone == 11)
       {
		   en->actor.pos.y = 55;
	   }
	   if (en->timerone == 12)
       {
		   en->actor.pos.y = 45;
	   }
	   if (en->timerone == 13)
       {
		   en->actor.pos.y = 35;
	   }
	   if (en->timerone == 14)
       {
		   en->actor.pos.y = 35;
		   en->timerone = 0;
		        sound_play_actor( &en->actor, 0x3844);
       }
	   
	   if (en->timertwo == 64)
       {
		   en->actor.pos.y = 35;
		   en->timerone = 0;
		   en->timertwo = 0;
		   en->attackchooser = 2;
       }

	   
}

if (en->attackchooser == 2) //Jump Slash
{
u32 anim;
	en->timerone += 1;
	en->timertwo += 1;

       z_actor_move_dir_vel(&en->actor); //move in direction
       actor_collision_routine(global, &en->actor, 15.0f, 28.0f, 60.0f, 0x1D);
       
	  
	   
       if (en->timerone == 1)
       {
		    en->actor.dir.y = en->actor.rot_toward_link_y;
       en->actor.rot.y = en->actor.rot_toward_link_y;
		   en->actor.xz_speed = 11.5f;
		   en->actor.pos.y = 35;
                  anim = ANIM_SIMPLESLASH;
                  en->anim = ANIM_SIMPLESLASH_ID;
                    z_skelanime_change_anim(
			        &en->skelanime, anim, 1.0,
			           0, 0, 0
		         );
       }
	   
	   if(en->actor.pos.y < 35)
	   {
		   en->actor.pos.y = 35;
	   }
	   
	   //Jump
	   if (en->timerone == 2)
       {
		    en->actor.pos.y = 45;		
	   }
	   
	    if (en->timerone == 3)
       {
		    en->actor.pos.y = 55;		
	   }
	   
	    if (en->timerone == 4)
       {
		    en->actor.pos.y = 65;		
	   }
	   
	    if (en->timerone == 5)
       {
		    en->actor.pos.y = 75;		
	   }
	   
	    if (en->timerone == 6)
       {
		    en->actor.pos.y = 85;		
	   }
	   
	    if (en->timerone == 7)
       {
		    en->actor.pos.y = 65;		
	   }
	   
	    if (en->timerone == 8)
       {
		    en->actor.pos.y = 45;		
	   }
	   
	   if (en->timerone == 9)
       {
		    en->actor.pos.y = 35;
			sound_play_actor( &en->actor, 0x3844);
			en->actor.xz_speed = 0.0f;
	   }

	   if (en->timerone == 11)
       {
		   en->actor.pos.y = 35;
		     anim = ANIM_SIMPLESLASHEND;
                  en->anim = ANIM_SIMPLESLASHEND_ID;
                    z_skelanime_change_anim(
			        &en->skelanime, anim, 1.0,
			           0, 0, 0
		         );
       }

       if (en->timerone == 13)
       {
		    sound_play_actor( &en->actor, 0x1801);	
	   }
	   
	   if (en->timerone == 19)
       {
		   en->timerone = 0;
		   en->timertwo = 0;
		   en->attackchooser = 3;
	   }
	   
}


if (en->attackchooser == 3) //Rotate
{
	u32 anim;
	en->timerone += 1;
	en->timertwo += 1;

       z_actor_move_dir_vel(&en->actor); //move in direction
       actor_collision_routine(global, &en->actor, 15.0f, 28.0f, 60.0f, 0x1D);
       
	   
	  
       en->actor.rot.y += 9999;
	   
       if (en->timerone == 1)
       {
		    en->actor.dir.y = en->actor.rot_toward_link_y;
		    en->actor.xz_speed = 15.5f;
		    en->actor.pos.y = 35;
                  anim = ANIM_SPIN;
                  en->anim = ANIM_SPIN_ID;
                    z_skelanime_change_anim(
			        &en->skelanime, anim, 1.0,
			           0, 0, 0
		         );
       }
	   
	
		en->actor.pos.y = 35;
	   
	   if (en->timerone == 11)
       {
		   en->timerone = 0;
		   en->timertwo = 0;
		   en->attackchooser = 4;
		   play_sound_global_once(0x386A);
		   
	   }
	   
}

if (en->attackchooser == 4) //Surprise Mother Fucker
{
		u32 anim;
	en->timerone += 1;
    en->timertwo += 1;

       z_actor_move_dir_vel(&en->actor); //move in direction
       actor_collision_routine(global, &en->actor, 15.0f, 28.0f, 60.0f, 0x1D);
       en->actor.xz_speed = 0.0f;
	   en->actor.dir.y = en->actor.rot_toward_link_y;
       en->actor.rot.y = en->actor.rot_toward_link_y;
	   
       if (en->timerone == 1)
       {
		   en->actor.pos.y = 35;
                  anim = ANIM_WALK;
                  en->anim = ANIM_WALK_ID;
                    z_skelanime_change_anim(
			        &en->skelanime, anim, 1.0,
			           0, 0, 0
		         );
       }
	   
	   if(en->actor.pos.y < 35)
	   {
		   en->actor.pos.y = 35;
	   }
	   
	   //Jump
	   if (en->timerone == 0)
       {
		    en->actor.pos.y = 35;
			actor_set_scale(&en->actor, 0.04f);
	   }
	   
	   if (en->timerone == 1)
       {
		   en->actor.pos.y = 35;
		   actor_set_scale(&en->actor, 0.04f);
	   }
		   
	   if (en->timerone == 2)
       {
		   en->actor.pos.y = 35;
		   actor_set_scale(&en->actor, 0.04f);
	   }
	   
	   if (en->timerone == 3)
       {
		   en->actor.pos.y = 45;
		   actor_set_scale(&en->actor, 0.035f);
	   }
	   
	   if (en->timerone == 4)
       {
		   en->actor.pos.y = 55;
		   actor_set_scale(&en->actor, 0.03f);
	   }
	   
	   if (en->timerone == 5)
       {
		   en->actor.pos.y = 65;
		   actor_set_scale(&en->actor, 0.025f);
	   }
	   
	   if (en->timerone == 6)
       {
		   en->actor.pos.y = 75;
		   actor_set_scale(&en->actor, 0.02f);
	   }
	   if (en->timerone == 7)
       {
		   en->actor.pos.y = 500;
		   actor_set_scale(&en->actor, 0.01f);
	   }
	   
	   //Fall
	   if (en->timerone == 18)
       {
		   en->actor.pos.y = 115;
		   en->actor.pos.x = Link.pos.x;
           en->actor.pos.z = Link.pos.z;
	   }
	   if (en->timerone == 19)
       {
		   en->actor.pos.y = 75;
		    actor_set_scale(&en->actor, 0.02f);
	   }
	   if (en->timerone == 20)
       {
		   en->actor.pos.y = 65;
		    actor_set_scale(&en->actor, 0.025f);
	   }	   
	   if (en->timerone == 21)
       {
		   en->actor.pos.y = 55;
		    actor_set_scale(&en->actor, 0.03f);
	   }
	   if (en->timerone == 22)
       {
		   en->actor.pos.y = 45;
		    actor_set_scale(&en->actor, 0.035f);
	   }
	   if (en->timerone == 23)
       {
		   en->actor.pos.y = 35;
		   actor_set_scale(&en->actor, 0.04f);
	   }
	   if (en->timerone == 24)
       {
		     en->actor.pos.y = 35;
		     anim = ANIM_SIMPLESLASHEND;
                  en->anim = ANIM_SIMPLESLASHEND_ID;
                    z_skelanime_change_anim(
			        &en->skelanime, anim, 1.0,
			           0, 0, 0
		         );
       }

       if (en->timerone == 26)
       {
		    sound_play_actor( &en->actor, 0x1801);	
	   }
	   
	   if (en->timertwo == 28)
       {
		   en->actor.pos.y = 35;
		   en->timerone = 0;
		   en->timertwo = 0;
		   en->attackchooser = 5;
       }
}
	
if (en->attackchooser == 5) //WHERE ARE YOU?
{
		u32 anim;
	en->timerone += 1;
    en->timertwo += 1;

       z_actor_move_dir_vel(&en->actor); //move in direction
       actor_collision_routine(global, &en->actor, 15.0f, 28.0f, 60.0f, 0x1D);
       en->actor.xz_speed = 0.0f;
	   
	   en->actor.pos.y = 35;
	   
       if (en->timerone == 1)
       {
		   
                  anim = ANIM_WAR;
                  en->anim = ANIM_WAR_ID;
                    z_skelanime_change_anim(
			        &en->skelanime, anim, 1.0,
			           0, 0, 0
		         );
       }

	   
	   if (en->timertwo == 36)
       {
		   en->actor.pos.y = 35;
		   en->timerone = 0;
		   en->timertwo = 0;
		   en->attackchooser = 1;
       }
}

if (en->actor.health == 0)
{
	
		
	en->attackchooser = 80;
    en->actor.xz_speed = 0.0f;
	
    u32 anim;
    en->timerdie += 1;

if (en->timerdie == 1)
{	
anim = ANIM_DIE;
sound_set_bgm(0x00);
en->anim = ANIM_DIE_ID;
        z_skelanime_change_anim(
			&en->skelanime, anim, 1.0,
			0, 0, 0
		);
}

if (en->timerdie == 80)
{	

sound_set_bgm(0x18);
anim = ANIM_DTC;
en->anim = ANIM_DTC_ID;
        z_skelanime_change_anim(
			&en->skelanime, anim, 1.0,
			0, 0, 0
		);
}

if (en->timerdie == 92)
{	
sound_set_bgm(0x18);
anim = ANIM_DIE_CROUCH;
en->anim = ANIM_DIE_CROUCH_ID;
        z_skelanime_change_anim(
			&en->skelanime, anim, 1.0,
			0, 0, 0
		);
}

if (en->timerdie == 150)
{	
actor_kill(&en->actor);
z64_actor_t *explosionInstance = actor_spawn(AADDR(global, 0x1C24), global, 0x0010, en->actor.pos.x, en->actor.pos.y, en->actor.pos.z, 0, 0, 0, 0x0000);
    AVAL(explosionInstance, uint16_t, 0x01F8) = 0;
}	}	
	
	//external_func_8002F2F4(&en->actor, global);

	//actor_set_height( &en->actor, 125.0f);//( &en->actor, 85.0f);
	
    zh_draw_debug_text(global, 0xFFFFFFFF, 5, 5, "BOB: %d", en->actor.health);
	zh_draw_debug_text(global, 0xFF0000FF, 5, 6, "X: %d", en->actor.pos.x);
	zh_draw_debug_text(global, 0x00FF00FF, 5, 7, "Y: %d", en->actor.pos.y);
	zh_draw_debug_text(global, 0x0000FFFF, 5, 8, "Z: %d", en->actor.pos.z);

     u8 prevhealth = en->actor.health;
    actor_update_health(&en->actor);
    if (en->actor.health != prevhealth)
    {
		
        en->actor.damage_color = 0x5FF1;
        sound_play_actor( &en->actor, 0x3956);
    }
	 if (en->actor.damage_effect == 0xF1) // Stun
    {
        en->actor.damage_color = 0x1FF1;

    }

actor_collision_routine(global, &en->actor, 15.0f, 28.0f, 60.0f, 0x1D);

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
			, 18
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
	.type = 0x05, 
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

