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
#include <Termin8or/ParticleSystem.h>


class Game : public GameEngine<>
{
  void update_lighting_rb_sprite(BitmapSprite* sprite, dynamics::RigidBody* rb,
                                 const RC& firesmoke_pos, float fire_light_radius_sq)
  {
    auto set_snowflake_color = [this](int rw, int cw, Color col)
    {
      auto it = snowflake_map.find({rw, cw});
      if (it == snowflake_map.end())
        return;
      for (auto* snowflake_sprite : it->second)
      {
        auto* snowflake_texture = dynamic_cast<BitmapSprite*>(snowflake_sprite)->get_curr_frame(get_anim_count(0));
        if (snowflake_texture != nullptr)
        {
          auto sf_textel = (*snowflake_texture)(0, 0);
          sf_textel.fg_color = col;
          sf_textel.bg_color = col;
          snowflake_texture->set_textel(0, 0, sf_textel);
        }
      }
    };
    
    auto gnd_lvl = sh.num_rows() - ground_height;
    auto moon_top_mid = sprite_moon->pos + RC { 1, 4 };
    auto moon_btm_mid = sprite_moon->pos + RC { 3, 4 };
    auto moon_mid_left = sprite_moon->pos + RC { 2, 1 };
    auto moon_mid_right = sprite_moon->pos + RC { 2, 8 };
    auto moon_centroid = to_RC_round(sprite_moon->calc_curr_centroid(get_anim_count(0)));
  
    auto pos = sprite->pos;
    for (int r = 0; r < sprite->get_size().r; ++r)
    {
      int rw = r + pos.r;
      for (int c = 0; c < sprite->get_size().c; ++c)
      {
        int cw = c + pos.c;
        auto* texture = sprite->get_curr_frame(get_anim_count(0));
        auto textel = texture->operator()(r, c);
        if (textel.bg_color == Color::DarkGreen || textel.bg_color == Color::Green)
        {
          auto moon_dir_center = math::normalize(to_Vec2({
            rw - moon_centroid.r,
            cw - moon_centroid.c }));
          textel.fg_color = Color::Green;
          textel.bg_color = Color::DarkGreen;
          set_snowflake_color(rw, cw, Color::LightGray);
          auto n = rb->fetch_surface_normal({ r, c });
          if ((n.r != 0.f && n.c != 0.f && math::dot(n, moon_dir_center) < -.8f)
              || math::distance_squared(rw*1.5f, static_cast<float>(cw), firesmoke_pos.r*1.5f, static_cast<float>(firesmoke_pos.c)) < fire_light_radius_sq)
          {
            textel.fg_color = Color::DarkGreen;
            textel.bg_color = Color::Green;
            set_snowflake_color(rw, cw, Color::White);
          }
          texture->set_textel(r, c, textel);
        }
        if (textel.bg_color != Color::Transparent2)
        {
          auto moon_dir_tm = math::normalize(to_Vec2({
            rw - moon_top_mid.r,
            cw - moon_top_mid.c}));
          auto moon_dir_bm = math::normalize(to_Vec2({
            rw - moon_btm_mid.r,
            cw - moon_btm_mid.c}));
          auto moon_dir_ml = math::normalize(to_Vec2({
            rw - moon_mid_left.r,
            cw - moon_mid_left.c}));
          auto moon_dir_mr = math::normalize(to_Vec2({
            rw - moon_mid_right.r,
            cw - moon_mid_right.c}));
          // End of shadow.
          auto t1 = math::lerp(static_cast<float>(std::sin(moon_angle)), 20.f, 2.f);
          auto draw_shadow_line = [t1, gnd_lvl, rw, cw, this](const Vec2& moon_dir)
          {
            auto gnd_rw1 = math::roundI(static_cast<float>(rw) + t1*moon_dir.r);
            auto gnd_cw1 = math::roundI(static_cast<float>(cw) + t1*moon_dir.c);
            // Start of shadow.
            // sh.num_rows() - ground_height = rw + t*moon_dir.r;
            auto t0 = (gnd_lvl - static_cast<float>(rw))/moon_dir.r;
            auto gnd_rw0 = math::roundI(static_cast<float>(rw) + t0*moon_dir.r);
            auto gnd_cw0 = math::roundI(static_cast<float>(cw) + t0*moon_dir.c);
            if (gnd_rw1 >= gnd_lvl && gnd_rw0 >= gnd_lvl)
              sprite_ground->plot_line(get_anim_count(0), { gnd_rw0, gnd_cw0 }, { gnd_rw1, gnd_cw1 },
                                       std::nullopt, std::nullopt, Color::DarkGray, std::nullopt);
          };
          draw_shadow_line(moon_dir_tm);
          draw_shadow_line(moon_dir_bm);
          draw_shadow_line(moon_dir_ml);
          draw_shadow_line(moon_dir_mr);
        }
      }
    }
  }

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
    sprite_ground->fill_sprite_chars(0, ' ');
    sprite_ground->fill_sprite_bg_colors(0, Color::LightGray);
    sprite_ground->fill_sprite_materials(0, 1);
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
    
  
    sprite_tree = sprh.create_bitmap_sprite("tree");
    sprite_tree->layer_id = 2;
    sprite_tree->init(10, 9);
    sprite_tree->pos = { sh.num_rows() - sprite_tree->get_size().r - ground_height, 5 };
    sprite_tree->create_frame(0);
    sprite_tree->set_sprite_chars_from_strings(0,
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
    sprite_tree->set_sprite_fg_colors(0,
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
    sprite_tree->set_sprite_bg_colors(0,
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
    sprite_tree->set_sprite_materials(0,
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
    sprite_tree->func_calc_anim_frame = [](int sim_frame) { return 0; };
    sprite_tree_arr = sprh.clone_sprite_array<20>("tree", "tree");
    sprite_tree->enabled = false;
    rb_tree_arr = dyn_sys.add_rigid_bodies<20>(sprite_tree_arr,
      [](int){ return 0.f; }, // mass
      [](int){ return std::nullopt; }, // pos, nullopt because static rigid objects.
      [](int){ return Vec2 {}; }, // vel
      [](int){ return Vec2 {}; }, // force
      [](int){ return 0.f; }, // ang_vel
      [](int){ return 0.f; }, // torque
      [this](int){ return e_tree; },
      [this](int){ return friction_tree; },
      [](int){ return 0.f; }, // crit speed
      [](int){ return std::vector { 1 }; }, // inertia mats
      [](int){ return std::vector { 1 }; } // coll mats
    );
    for (auto* sprite : sprite_tree_arr)
    {
      auto* bmp_sprite = static_cast<BitmapSprite*>(sprite);
      bmp_sprite->pos.c = rnd::rand_float(0.f, 160.f);
      if (rnd::one_in(3))
        bmp_sprite->flip_lr(0);
    }
    
    sprite_fireplace = sprh.create_bitmap_sprite("fireplace");
    sprite_fireplace->layer_id = 2;
    sprite_fireplace->init(2, 8);
    sprite_fireplace->pos = { sh.num_rows() - ground_height + 1, 35 };
    sprite_fireplace->create_frame(0);
    sprite_fireplace->set_sprite_chars_from_strings(0,
      R"( //Oo\\ )",
      R"(/()==\O))"
    );
    sprite_fireplace->set_sprite_fg_colors(0,
      Color::Transparent2, Color::DarkRed, Color::Red, Color::DarkRed, Color::DarkRed, Color::Red, Color::DarkRed, Color::Transparent2,
      Color::DarkRed, Color::Red, Color::Red, Color::Red, Color::Red, Color::Red, Color::Red, Color::DarkRed
    );
    sprite_fireplace->set_sprite_bg_colors(0,
      Color::Transparent2, Color::Transparent2, Color::DarkRed, Color::Black, Color::Black, Color::DarkRed, Color::Transparent2, Color::Transparent2,
      Color::Transparent2, Color::DarkRed, Color::DarkRed, Color::DarkRed, Color::DarkRed, Color::DarkRed, Color::DarkRed, Color::Transparent2
    );
    
    smoke_color_gradients.emplace_back(0.5f, smoke_0);
    smoke_color_gradients.emplace_back(0.6f, smoke_1);
    
    sprite_snowflake = sprh.create_bitmap_sprite("snowflake");
    sprite_snowflake->layer_id = 3;
    sprite_snowflake->pos = { 0, 27 };
    sprite_snowflake->init(1, 1);
    sprite_snowflake->create_frame(0);
    sprite_snowflake->set_sprite_chars(0, '*');
    sprite_snowflake->set_sprite_fg_colors(0, Color::White);
    sprite_snowflake->set_sprite_bg_colors(0, Color::Transparent2);
    sprite_snowflake->set_sprite_materials(0, 1);
    f_snowflake_vel = [](int){ return Vec2 { rnd::rand_float(0.4f, 0.6f), rnd::rand_float(-4.f, -2.f)}; };
    dyn_sys.add_rigid_body(sprite_snowflake, .5f, std::nullopt,
      { 0.5f, -3.f }, { 0.1f, 0.12f },
      0.f, 0.f,
      e_snowflake, friction_snowflake);
    sprite_snowflake_arr = sprh.clone_sprite_array<1000>("snowflake", "snowflake");
    sprite_snowflake->enabled = false;
    rb_snowflake_arr = dyn_sys.add_rigid_bodies<1000>(sprite_snowflake_arr,
      [](int){ return 0.5f; },
      [](int){ return Vec2 { rnd::rand_float(-500.f, 0.f), rnd::rand_float(-2.f, 81.f) }; }, // pos
      f_snowflake_vel, // vel
      [](int){ return Vec2 { 0.1f, rnd::rand_float(0.f, 0.2f) }; }, // force
      [](int){ return 0.f; }, [](int){ return 0.f; },
      [this](int){ return e_snowflake; }, [this](int){ return friction_snowflake; },
      [](int){ return rnd::rand_float(0.2f, 3.f); });
    for (auto* rb : rb_snowflake_arr)
      rb->set_sleeping(true,
                       0.05f, 0.5f, // vel, time
                       10.f); // impulse
    
    
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
    
    coll_handler.exclude_all_rigid_bodies_of_prefix(&dyn_sys, "tree", "tree");
    coll_handler.rebuild_BVH(sh.num_rows(), sh.num_cols(), &dyn_sys);
  }
  
private:

  SpriteHandler sprh;
  dynamics::DynamicsSystem dyn_sys;
  dynamics::CollisionHandler coll_handler;
  
  const int ground_height = 5;
  BitmapSprite* sprite_ground = nullptr;
  dynamics::RigidBody* rb_ground = nullptr;
  
  BitmapSprite* sprite_tree = nullptr;
  std::array<Sprite*, 20> sprite_tree_arr;
  std::array<dynamics::RigidBody*, 20> rb_tree_arr;

  BitmapSprite* sprite_moon = nullptr;
  
  BitmapSprite* sprite_fireplace = nullptr;
    
  BitmapSprite* sprite_snowflake = nullptr;
  std::array<Sprite*, 1000> sprite_snowflake_arr;
  std::array<dynamics::RigidBody*, 1000> rb_snowflake_arr;
  std::map<RC, std::vector<Sprite*>> snowflake_map;
  
  std::function<Vec2(int)> f_snowflake_vel;
  
  ParticleHandler fire_smoke_engine { 500 };
  
  ParticleGradientGroup smoke_0
  {
    {
      {
        { 0.00f, Color::White },
        { 0.27f, Color::Red },
        { 0.60f, Color::Yellow },
        { 0.75f, Color::LightGray },
        { 0.88f, Color::DarkGray },
      }
    },
    {
      {
        { 0.12f, Color::Blue },
        { 0.20f, Color::White },
        { 0.30f, Color::Yellow },   // 0.125f
        { 0.55f, Color::DarkRed },  // 0.375f
        { 0.70f, Color::DarkGray }, // 0.625f
        { 0.85f, Color::Black },    // 0.875f
      }
    },
    {
      {
        { 0.0000f, "o" },
        { 0.25f, "v" },
        { 0.45f, "s" },
        { 0.65f, "%" },
        { 0.6667f, "&" },
        { 0.8333f, "@" }
      }
    }
  };
  
  ParticleGradientGroup smoke_1
  {
    {
      {
        { 0.10f, Color::White },
        { 0.37f, Color::Red },
        { 0.70f, Color::Yellow },
        { 0.85f, Color::LightGray },
        { 0.98f, Color::DarkGray },
      }
    },
    {
      {
        { 0.12f, Color::Blue },
        { 0.30f, Color::White },
        { 0.50f, Color::Yellow },   // 0.125f
        { 0.74f, Color::DarkRed },  // 0.375f
        { 0.86f, Color::DarkGray }, // 0.625f
        { 1.00f, Color::Black },    // 0.875f
      }
    },
    {
      {
        { 0.0000f, "." },
        { 0.1667f, "*" },
        { 0.3333f, "s" },
        { 0.5000f, "%" },
        { 0.6667f, "&" },
        { 0.8333f, "@" }
      }
    }
  };
  std::vector<std::pair<float, ParticleGradientGroup>> smoke_color_gradients;
  
  const float smoke_life_time = 1.1f;
  const float smoke_spread = 3.f;
  const int smoke_cluster_size = 10;
  const float smoke_vel_r = -2.5f;
  const float smoke_vel_c = 0.1f;
  const float smoke_acc = -1.5f;
  
  const Vec2 moon_pivot = { 30.f, 37.f };
  const float moon_w = 5e-4f * math::c_2pi;
  const float moon_angle0 = math::deg2rad(7.5f);
  float moon_angle = 0.f;
  
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
  
  virtual void update() override
  {
    auto firesmoke_pos = sprite_fireplace->pos + RC { 0, sprite_fireplace->get_size().c/2 };
    auto fire_light_radius_sq = math::sq(math::linmap_clamped(get_sim_time_s(), 0.f, 4.f, 0.f, 10.f));
    fire_smoke_engine.update(firesmoke_pos, true, smoke_vel_r, smoke_vel_c, smoke_acc, smoke_spread, smoke_life_time, smoke_cluster_size, get_sim_dt_s(), get_sim_time_s());
    fire_smoke_engine.draw(sh, smoke_color_gradients, get_sim_time_s());
  
    if (use_dynamics_system)
    {
      dyn_sys.update(get_sim_time_s(), get_sim_dt_s(), get_anim_count(0));
      coll_handler.update();
    }

    float t = get_sim_time_s();
    moon_angle = moon_w*t + moon_angle0;
    sprite_moon->pos = to_RC_round({
      moon_pivot.r - 25.f*std::sin(moon_angle),
      moon_pivot.c + 30.f*std::cos(moon_angle)
    });
    
    sprite_ground->fill_sprite_bg_colors(0,
      math::in_range<float>(math::rad2deg(moon_angle), 18.f, 162.f, Range::Open) ? Color::LightGray : Color::DarkGray);
    for (size_t tree_idx = 0; tree_idx < sprite_tree_arr.size(); ++tree_idx)
      update_lighting_rb_sprite(static_cast<BitmapSprite*>(sprite_tree_arr[tree_idx]),
                                rb_tree_arr[tree_idx], firesmoke_pos, fire_light_radius_sq);
    
    for (auto* rb : rb_snowflake_arr)
    {
      if (rb->get_curr_cm().r >= sh.num_rows() - ground_height - 1)
      {
        rb->reset_curr_cm();
        rb->set_curr_lin_vel(f_snowflake_vel(0));
      }
        
      if (rb->is_sleeping())
      {
        auto* sprite = dynamic_cast<BitmapSprite*>(rb->get_sprite());
        if (sprite != nullptr)
        {
          auto* texture = sprite->get_curr_frame(get_anim_count(0));
          if (texture != nullptr)
          {
            auto curr_textel = (*texture)(0, 0);
            if (curr_textel.ch != '#')
            {
              curr_textel.ch = '#';
              curr_textel.fg_color = Color::LightGray;
              curr_textel.bg_color = Color::LightGray;
              texture->set_textel(0, 0, curr_textel);
              snowflake_map[sprite->pos].emplace_back(sprite);
            }
          }
        }
      }
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
