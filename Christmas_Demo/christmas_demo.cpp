//
//  christmas_demo.cpp
//  Christmas_Demo
//
//  Created by Rasmus Anthin on 2024-11-07.
//

#include <Termin8or/GameEngine.h>
#include <Termin8or/SpriteHandler.h>
#include <Termin8or/Dynamics/RigidBody.h>
#include <Termin8or/Dynamics/DynamicsSystem.h>
#include <Termin8or/Dynamics/CollisionHandler.h>


class Game : public GameEngine<>
{
public:
  Game(int argc, char** argv, const GameEngineParams& params)
    : GameEngine(argv[0], params)
  {
    GameEngine::set_anim_rate(0, 4);
  }
  
  virtual void generate_data() override
  {
    // Black,         //  1
    // DarkRed,       //  2
    // DarkGreen,     //  3
    // DarkYellow,    //  4
    // DarkBlue,      //  5
    // DarkMagenta,   //  6
    // DarkCyan,      //  7
    // LightGray,     //  8
    // DarkGray,      //  9
    // Red,           // 10
    // Green,         // 11
    // Yellow,        // 12
    // Blue,          // 13
    // Magenta,       // 14
    // Cyan,          // 15
    // White          // 16
  
    sprite_ground = sprh.create_bitmap_sprite("ground");
    sprite_ground->layer_id = 0;
    const int ground_height = 3;
    sprite_ground->pos = { sh.num_rows() - ground_height, 0 };
    sprite_ground->init(ground_height, sh.num_cols());
    sprite_ground->create_frame(0);
    sprite_ground->set_sprite_chars_from_strings(0,
      str::rep_char(' ', sh.num_cols()),
      str::rep_char(' ', sh.num_cols()),
      str::rep_char(' ', sh.num_cols())
    );
    sprite_ground->set_sprite_bg_colors(0,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16
    );
    sprite_ground->set_sprite_materials(0,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    );
    rb_ground = dyn_sys.add_rigid_body(sprite_ground, 0.f); // Zero mass == immovable.
    
    sprite_moon = sprh.create_bitmap_sprite("moon");
    sprite_moon->layer_id = 1;
    sprite_moon->pos = { 15, 63 };
    sprite_moon->init(5, 10);
    sprite_moon->create_frame(0);
    sprite_moon->set_sprite_chars_from_strings(0,
      "   ####   ",
      " O##O#### ",
      "##o#p##o##",
      " ##o##### ",
      "   #o##   "
    );
    sprite_moon->set_sprite_fg_colors(0,
      -2, -2, -2, 12, 12, 12, 12, -2, -2, -2,
      -2,  4, 12, 12,  4, 12, 12, 12, 12, -2,
      12, 12,  4, 12,  4, 12, 12,  4, 12, 12,
      -2, 12, 12,  4, 12, 12, 12, 12, 12, -2,
      -2, -2, -2, 12,  4, 12, 12, -2, -2, -2
    );
    sprite_moon->set_sprite_bg_colors(0,
      -2, -2, -2, 12, 12, 12, 12, -2, -2, -2,
      -2, 12, 12, 12, 12, 12, 12, 12, 12, -2,
      12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
      -2, 12, 12, 12, 12, 12, 12, 12, 12, -2,
      -2, -2, -2, 12, 12, 12, 12, -2, -2, -2
    );
    
  
    sprite_tree0 = sprh.create_bitmap_sprite("tree0");
    sprite_tree0->layer_id = 2;
    sprite_tree0->pos = { 17, 5 };
    sprite_tree0->init(10, 9);
    sprite_tree0->create_frame(0);
    sprite_tree0->set_sprite_chars_from_strings(0,
      R"(    ^    )",
      R"(   //\   )",
      R"(  //\\\  )",
      R"(  ///\\  )",
      R"( '//\\\. )",
      R"( ///}}\\ )",
      R"(////|){\\)",
      R"(/:/ | \\\)",
      R"(    |    )",
      R"(    |    )"
    );
    sprite_tree0->set_sprite_fg_colors(0,
       0,  0,  0,  0, 11,  0,  0,  0, 0,
       0,  0,  0, 11, 11,  3,  0,  0, 0,
       0,  0, 11, 11, 11, 11,  3,  0, 0,
       0,  0, 11, 11, 11, 11,  3,  0, 0,
       0, 11, 11, 11, 11, 11, 11,  3, 0,
       0, 11, 11, 11, 11, 11, 11,  3, 0,
      11, 11, 11, 11, 11, 11, 11, 11, 3,
      11, 11, 11,  0, 10,  0, 11, 11, 3,
       0,  0,  0,  0, 10,  0,  0,  0, 0,
       0,  0,  0,  0, 10,  0,  0,  0, 0
    );
    sprite_tree0->set_sprite_bg_colors(0,
      -2, -2, -2, -2,  3, -2, -2, -2, -2,
      -2, -2, -2,  3,  3, 11, -2, -2, -2,
      -2, -2,  3,  3,  3,  3, 11, -2, -2,
      -2, -2,  3,  3,  3,  3, 11, -2, -2,
      -2,  3,  3,  3,  3,  3,  3, 11, -2,
      -2,  3,  3,  3,  3,  3,  3, 11, -2,
       3,  3,  3,  3,  3,  3,  3,  3, 11,
       3,  3,  3, -2,  2, -2,  3,  3, 11,
      -2, -2, -2, -2,  2, -2, -2, -2, -2,
      -2, -2, -2, -2,  2, -2, -2, -2, -2
    );
    sprite_tree0->set_sprite_materials(0,
       0,  0,  0,  0,  1,  0,  0,  0,  0,
       0,  0,  0,  1,  1,  1,  0,  0,  0,
       0,  0,  1,  1,  1,  1,  1,  0,  0,
       0,  0,  1,  1,  1,  1,  1,  0,  0,
       0,  1,  1,  1,  1,  1,  1,  1,  0,
       0,  1,  1,  1,  1,  1,  1,  1,  0,
       1,  1,  1,  1,  1,  1,  1,  1,  1,
       1,  1,  1,  0,  1,  0,  1,  1,  1,
       0,  0,  0,  0,  1,  0,  0,  0,  0,
       0,  0,  0,  0,  1,  0,  0,  0,  0
    );
    sprite_tree0->func_calc_anim_frame = [](int sim_frame) { return 0; };
    
    sprite_tree1 = dynamic_cast<BitmapSprite*>(sprh.clone_sprite("tree1", "tree0"));
    sprite_tree1->pos.c = 33;
    
    sprite_tree2 = dynamic_cast<BitmapSprite*>(sprh.clone_sprite("tree2", "tree0"));
    sprite_tree2->pos.c = 68;
    
    sprite_snowflake = sprh.create_bitmap_sprite("snowflake");
    sprite_snowflake->layer_id = 3;
    sprite_snowflake->pos = { 0, 40 };
    sprite_snowflake->init(1, 1);
    sprite_snowflake->create_frame(0);
    sprite_snowflake->set_sprite_chars(0, '#');
    sprite_snowflake->set_sprite_fg_colors(0, Color::White);
    sprite_snowflake->set_sprite_bg_colors(0, Color::White);
    sprite_snowflake->set_sprite_materials(0, 1);
    
    rb_snowflake = dyn_sys.add_rigid_body(sprite_snowflake, .5f, { 0.5f, -3.f }, { 0.1f, 0.12f });
    
    
    for (int s_idx = 0; s_idx < 30; ++s_idx)
    {
      auto* sprite_star = sprh.create_bitmap_sprite("star" + std::to_string(s_idx));
      sprite_star->pos.r = rnd::rand_int(0, sh.num_rows()-1);
      sprite_star->pos.c = rnd::rand_int(0, sh.num_cols()-1);
      sprite_star->layer_id = 0;
      sprite_star->init(1, 1);
      sprite_star->create_frame(0);
      char star_ch = rnd::rand_select<char>({ '.', '+' });
      sprite_star->set_sprite_chars(0, star_ch);
      sprite_star->set_sprite_fg_colors(0, rnd::rand_select<Color>({ Color::White, Color::White, Color::White, Color::White, Color::White, Color::Yellow, Color::Yellow, Color::Yellow, Color::Red, Color::Blue, Color::Blue, Color::Blue }));
      sprite_star->set_sprite_bg_colors(0, Color::Transparent2);
      sprite_star->create_frame(1);
      sprite_star->set_sprite_chars(1, star_ch);
      sprite_star->set_sprite_fg_colors(1, Color::Black);
      sprite_star->set_sprite_bg_colors(1, Color::Transparent2);
      const int max_twinkle = 100;
      int twinkle_offs = rnd::rand_int(0, max_twinkle);
      sprite_star->func_calc_anim_frame = [twinkle_offs](int sim_frame)
      {
        if ((sim_frame + twinkle_offs) % max_twinkle == 0)
          return 1;
        return 0;
      };
    }
    
    coll_handler.rebuild_BVH(sh.num_rows(), sh.num_cols(), &dyn_sys);
  }
  
private:

  SpriteHandler sprh;
  dynamics::DynamicsSystem dyn_sys;
  dynamics::CollisionHandler coll_handler;
  
  BitmapSprite* sprite_ground = nullptr;
  dynamics::RigidBody* rb_ground = nullptr;
  
  BitmapSprite* sprite_tree0 = nullptr;
  BitmapSprite* sprite_tree1 = nullptr;
  BitmapSprite* sprite_tree2 = nullptr;
  BitmapSprite* sprite_moon = nullptr;
  
  BitmapSprite* sprite_snowflake = nullptr;
  dynamics::RigidBody* rb_snowflake = nullptr;
  
  bool use_dynamics_system = true;
  bool dbg_draw_rigid_bodies = false;
  bool dbg_draw_sprites = false;
  bool draw_sprites = true;
  bool dbg_draw_broad_phase = false;
  bool dbg_draw_narrow_phase = false;
  
  virtual void update() override
  {
    if (use_dynamics_system)
    {
      dyn_sys.update(get_sim_dt_s(), get_anim_count(0));
      coll_handler.update();
    }
  
    if (dbg_draw_rigid_bodies)
      dyn_sys.draw_dbg(sh);
    if (dbg_draw_sprites)
      sprh.draw_dbg_pts(sh, get_anim_count(0));
    if (dbg_draw_narrow_phase)
      coll_handler.draw_dbg_narrow_phase(sh);
    if (draw_sprites)
      sprh.draw(sh, get_anim_count(0));
    if (dbg_draw_sprites)
      sprh.draw_dbg_bb(sh, get_anim_count(0));
    if (dbg_draw_broad_phase)
      coll_handler.draw_dbg_broad_phase(sh, 0);
  }
};

int main(int argc, char** argv)
{
  GameEngineParams params;
  params.screen_bg_color_default = Color::Black;
  params.enable_title_screen = false;
  params.enable_instructions_screen = false;
  
  Game game(argc, argv, params);
  
  game.init();
  game.generate_data();
  game.run();

  return EXIT_SUCCESS;
}
