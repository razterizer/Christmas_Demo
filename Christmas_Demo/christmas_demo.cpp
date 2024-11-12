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
    sprite_ground->layer_id = 1;
    sprite_ground->pos = { sh.num_rows() - ground_height, 0 };
    sprite_ground->init(ground_height, sh.num_cols());
    sprite_ground->create_frame(0);
    sprite_ground->set_sprite_chars_from_strings(0,
      str::rep_char(' ', sh.num_cols()),
      str::rep_char(' ', sh.num_cols()),
      str::rep_char(' ', sh.num_cols())
    );
    sprite_ground->set_sprite_bg_colors(0,
      8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
      8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
      8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
    );
    sprite_ground->set_sprite_materials(0,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    );
    rb_ground = dyn_sys.add_rigid_body(sprite_ground, 0.f, // Zero mass == immovable.
      std::nullopt, {}, {},
      0.f, 0.f,
      e_ground, friction_ground);
    
    sprite_moon = sprh.create_bitmap_sprite("moon");
    sprite_moon->layer_id = 0;
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
    rb_tree0 = dyn_sys.add_rigid_body(sprite_tree0, 0.f, std::nullopt, {}, {}, 0.f, 0.f, e_tree, friction_tree);
    
    sprite_tree1 = dynamic_cast<BitmapSprite*>(sprh.clone_sprite("tree1", "tree0"));
    sprite_tree1->pos.c = 33;
    rb_tree1 = dyn_sys.add_rigid_body(sprite_tree1, 0.f, std::nullopt, {}, {}, 0.f, 0.f, e_tree, friction_tree);
    
    sprite_tree2 = dynamic_cast<BitmapSprite*>(sprh.clone_sprite("tree2", "tree0"));
    sprite_tree2->pos.c = 68;
    sprite_tree2->flip_lr(0);
    rb_tree2 = dyn_sys.add_rigid_body(sprite_tree2, 0.f, std::nullopt, {}, {}, 0.f, 0.f, e_tree, friction_tree);
    
    sprite_snowflake_c = sprh.create_bitmap_sprite("snowflake_c");
    sprite_snowflake_c->layer_id = 3;
    sprite_snowflake_c->pos = { 0, 27 };
    sprite_snowflake_c->init(1, 1);
    sprite_snowflake_c->create_frame(0);
    sprite_snowflake_c->set_sprite_chars(0, '*');
    sprite_snowflake_c->set_sprite_fg_colors(0, Color::White);
    sprite_snowflake_c->set_sprite_bg_colors(0, Color::Transparent2);
    sprite_snowflake_c->set_sprite_materials(0, 1);
    dyn_sys.add_rigid_body(sprite_snowflake_c, .5f, std::nullopt,
      { 0.5f, -3.f }, { 0.1f, 0.12f },
      0.f, 0.f,
      e_snowflake, friction_snowflake);
    snowflakes_coll = sprh.clone_sprite_array<1000>("snowflake_c", "snowflake_c");
    rb_snowflakes_coll = dyn_sys.add_rigid_bodies<1000>(snowflakes_coll,
      [](int){ return 0.5f; },
      [](int){ return Vec2 { rnd::rand_float(-500.f, 0.f), rnd::rand_float(-2.f, 81.f) }; }, // pos
      [](int){ return Vec2 { rnd::rand_float(0.4f, 0.6f), rnd::rand_float(-4.f, -2.f)}; }, // vel
      [](int){ return Vec2 { 0.1f, rnd::rand_float(0.f, 0.2f) }; }, // force
      [](int){ return 0.f; }, [](int){ return 0.f; },
      [this](int){ return e_snowflake; }, [this](int){ return friction_snowflake; });
    
    //sprite_snowflake_nc = sprh.create_bitmap_sprite("snowflake_nc");
    //sprite_snowflake_nc->layer_id = 3;
    //sprite_snowflake_nc->pos = { 0, 50 };
    //sprite_snowflake_nc->init(1, 1);
    //sprite_snowflake_nc->create_frame(0);
    //sprite_snowflake_nc->set_sprite_chars(0, '*');
    //sprite_snowflake_nc->set_sprite_fg_colors(0, Color::White);
    //sprite_snowflake_nc->set_sprite_bg_colors(0, Color::Transparent2);
    //sprite_snowflake_nc->set_sprite_materials(0, 1);
    //dyn_sys.add_rigid_body(sprite_snowflake_nc, .5f, { 0.5f, -3.f }, { 0.1f, 0.12f }, 0.f, 0.f, e, friction, { 1 }, {});
    //snowflakes_non_coll = sprh.clone_sprite_array<50>("snowflake_nc", "snowflake_nc");
    
    
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
  
  const int ground_height = 3;
  BitmapSprite* sprite_ground = nullptr;
  dynamics::RigidBody* rb_ground = nullptr;
  
  BitmapSprite* sprite_tree0 = nullptr;
  dynamics::RigidBody* rb_tree0 = nullptr;
  BitmapSprite* sprite_tree1 = nullptr;
  dynamics::RigidBody* rb_tree1 = nullptr;
  BitmapSprite* sprite_tree2 = nullptr;
  dynamics::RigidBody* rb_tree2 = nullptr;
  
  BitmapSprite* sprite_moon = nullptr;
  
  BitmapSprite* sprite_snowflake_c = nullptr;
  std::array<Sprite*, 1000> snowflakes_coll;
  std::array<dynamics::RigidBody*, 1000> rb_snowflakes_coll;
  //BitmapSprite* sprite_snowflake_nc = nullptr;
  //std::array<Sprite*, 50> snowflakes_non_coll;
  
  float e_ground = 0.05f;
  float e_tree = 0.1f;
  float e_snowflake = 0.07f;
  float friction_ground = 0.5f;
  float friction_tree = 0.95f;
  float friction_snowflake = 0.8f;
  
  bool use_dynamics_system = true;
  bool dbg_draw_rigid_bodies = false;
  bool dbg_draw_sprites = false;
  bool draw_sprites = true;
  bool dbg_draw_broad_phase = false;
  bool dbg_draw_narrow_phase = false;
  
  void update_lighting_rb_sprite(BitmapSprite* sprite, dynamics::RigidBody* rb)
  {
    for (int r = 0; r < sprite->get_size().r; ++r)
    {
      for (int c = 0; c < sprite->get_size().c; ++c)
      {
        auto* texture = sprite->get_curr_frame(get_anim_count(0));
        auto textel = texture->operator()(r, c);
        if (textel.bg_color == Color::DarkGreen || textel.bg_color == Color::Green)
        {
          textel.fg_color = Color::Green;
          textel.bg_color = Color::DarkGreen;
          auto n = rb->fetch_surface_normal({ r, c });
          if (n.r != 0.f && n.c != 0.f)
          {
            auto moon_dir = math::normalize(to_Vec2({
              sprite->pos.r + r - sprite_moon->pos.r,
              sprite->pos.c + c - sprite_moon->pos.c }));
            if (math::dot(n, moon_dir) < -.7f)
            {
              textel.fg_color = Color::DarkGreen;
              textel.bg_color = Color::Green;
            }
          }
          texture->set_textel(r, c, textel);
        }
      }
    }
  }
  
  virtual void update() override
  {
    if (use_dynamics_system)
    {
      dyn_sys.update(get_sim_dt_s(), get_anim_count(0));
      coll_handler.update();
    }
    
    const Vec2& pivot = { 30.f, 35.f };
    const float w = 5e-4f * math::c_2pi;
    const float alpha0 = math::deg2rad(10.f);
    float t = get_sim_time_s();
    float alpha = w*t + alpha0;
    sprite_moon->pos = to_RC_round({ pivot.r - 25.f*std::sin(alpha), pivot.c + 30.f*std::cos(alpha) });
    
    update_lighting_rb_sprite(sprite_tree0, rb_tree0);
    update_lighting_rb_sprite(sprite_tree1, rb_tree1);
    update_lighting_rb_sprite(sprite_tree2, rb_tree2);
  
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
