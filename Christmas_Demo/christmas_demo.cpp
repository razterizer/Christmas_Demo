//
//  christmas_demo.cpp
//  Christmas_Demo
//
//  Created by Rasmus Anthin on 2024-11-07.
//

#include <Termin8or/sys/GameEngine.h>
#include <Termin8or/sprite/SpriteHandler.h>
#include <Termin8or/physics/dynamics/RigidBody.h>
#include <Termin8or/physics/dynamics/DynamicsSystem.h>
#include <Termin8or/physics/dynamics/CollisionHandler.h>
#include <Termin8or/physics/ParticleSystem.h>
#include <Termin8or/title/ASCII_Fonts.h>
#include <Termin8or/drawing/Animation.h>
#include <8Beat/AudioSourceHandler.h>
#include <8Beat/WaveformHelper.h>
#include <8Beat/ChipTuneEngine.h>
#include <8Beat/ChipTuneEngineListener.h>
#include <Core/Benchmark.h>

using RC = t8::RC;
using Color16 = t8::Color16;
using Color = t8::Color;
using Sprite = t8x::Sprite;
using BitmapSprite = t8x::BitmapSprite;
using SpriteHandler = t8x::SpriteHandler;
using TransitionAnimationSingle = t8x::TransitionAnimationSingle;
using TransitionAnimationInOut = t8x::TransitionAnimationInOut;
using ParticleHandler = t8x::ParticleHandler;
using ParticleGradientGroup = t8x::ParticleGradientGroup;
using RigidBody = t8x::RigidBody;
using DynamicsSystem = t8x::DynamicsSystem;
using CollisionHandler = t8x::CollisionHandler;

static const float c_default_vol = 0.5f;


class Game : public t8x::GameEngine<>, public beat::ChipTuneEngineListener
{
  void play_tune(const std::string& tune_filename)
  {
    try
    {
      std::string tune_path = get_exe_folder();
#ifndef _WIN32
      const char* xcode_env = std::getenv("RUNNING_FROM_XCODE");
      if (xcode_env != nullptr)
        tune_path = "../../../../../../../../Documents/xcode/Christmas_Demo/Christmas_Demo/"; // #FIXME: Find a better solution!
#endif
      
      if (chip_tune->load_tune(folder::join_path({ tune_path, tune_filename })))
      {
        chip_tune->play_tune_async();
        //chip_tune->wait_for_completion();
      }
    }
    catch (const std::exception& e)
    {
      std::cerr << "Caught exception: " << e.what() << std::endl;
    }
  }
  
  void generate_star_sprites()
  {
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
      sprite_star->set_sprite_fg_colors(0, rnd::rand_select<Color16>({ Color16::White, Color16::White, Color16::White, Color16::White, Color16::White, Color16::Yellow, Color16::Yellow, Color16::Yellow, Color16::Red, Color16::Blue, Color16::Blue, Color16::Blue }));
      sprite_star->set_sprite_bg_colors(0, Color16::Transparent2);
      sprite_star->create_frame(1);
      sprite_star->set_sprite_chars(1, star_ch);
      sprite_star->set_sprite_fg_colors(1, Color16::Black);
      sprite_star->set_sprite_bg_colors(1, Color16::Transparent2);
      const int max_twinkle = 100;
      int twinkle_offs = rnd::rand_int(0, max_twinkle);
      sprite_star->func_calc_anim_frame = [twinkle_offs](int sim_frame)
      {
        if ((sim_frame + twinkle_offs) % max_twinkle == 0)
          return 1;
        return 0;
      };
    }
  }

  void update_lighting_rb_sprite(BitmapSprite* sprite, RigidBody* rb,
                                 const t8::Style& dark_style,
                                 bool use_fire, const RC& firesmoke_pos, float fire_light_radius_sq,
                                 bool is_moon_up, bool casts_shadow)
  {
    auto light_style = t8::shade_style(dark_style, t8::ShadeType::Bright, true);
  
    auto set_snowflake_color = [this](int rw, int cw, Color col)
    {
      auto it = snowflake_map.find({rw, cw});
      if (it == snowflake_map.end())
        return;
      for (auto* snowflake_sprite : it->second)
      {
        auto* snowflake_texture = dynamic_cast<BitmapSprite*>(snowflake_sprite)->get_curr_sim_frame(get_anim_count(0));
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
    auto moon_centroid = t8::to_RC_round(sprite_moon->calc_curr_centroid(get_anim_count(0)));
  
    auto pos = sprite->pos;
    for (int r = 0; r < sprite->get_size().r; ++r)
    {
      int rw = r + pos.r;
      for (int c = 0; c < sprite->get_size().c; ++c)
      {
        int cw = c + pos.c;
        auto* texture = sprite->get_curr_sim_frame(get_anim_count(0));
        auto textel = texture->operator()(r, c);
        if (textel.bg_color == dark_style.bg_color || textel.bg_color == light_style.bg_color)
        {
          auto moon_dir_center = math::normalize(t8::to_Vec2({
            rw - moon_centroid.r,
            cw - moon_centroid.c }));
          textel.set_style(dark_style);
          set_snowflake_color(rw, cw, Color16::LightGray);
          auto n = rb->fetch_surface_normal({ r, c });
          if ((n.r != 0.f && n.c != 0.f && math::dot(n, moon_dir_center) < -0.2f && is_moon_up)
              || (use_fire && math::distance_squared(rw*1.5f, static_cast<float>(cw), firesmoke_pos.r*1.5f, static_cast<float>(firesmoke_pos.c)) < fire_light_radius_sq))
          {
            textel.set_style(light_style);
            set_snowflake_color(rw, cw, Color16::White);
          }
          texture->set_textel(r, c, textel);
        }
        if (textel.bg_color != Color16::Transparent2 && is_moon_up && casts_shadow)
        {
          auto moon_opaque_pts = sprite_moon->get_opaque_points(get_anim_count(0));
          auto t1 = math::lerp(static_cast<float>(std::sin(moon_angle)), 20.f, 2.f);
          auto draw_shadow_line = [t1, gnd_lvl, rw, cw, this](const RC& moon_pt)
          {
            auto moon_dir = math::normalize(t8::to_Vec2({ rw - moon_pt.r, cw - moon_pt.c }));
            auto gnd_rw1 = math::roundI(static_cast<float>(rw) + t1*moon_dir.r);
            auto gnd_cw1 = math::roundI(static_cast<float>(cw) + t1*moon_dir.c);
            // Start of shadow.
            // sh.num_rows() - ground_height = rw + t*moon_dir.r;
            auto t0 = (gnd_lvl - static_cast<float>(rw))/moon_dir.r;
            auto gnd_rw0 = math::roundI(static_cast<float>(rw) + t0*moon_dir.r);
            auto gnd_cw0 = math::roundI(static_cast<float>(cw) + t0*moon_dir.c);
            if (gnd_rw1 >= gnd_lvl && gnd_rw0 >= gnd_lvl)
              sprite_ground->plot_line(get_anim_count(0), { gnd_rw0, gnd_cw0 }, { gnd_rw1, gnd_cw1 },
                                       std::nullopt, std::nullopt,
                                       std::nullopt, std::nullopt,
                                       ground_shadow_color, std::nullopt,
                                       std::nullopt, std::nullopt);
          };
          for (const auto& moon_pt : moon_opaque_pts)
            draw_shadow_line(moon_pt);
        }
      }
    }
  }
  
  void update_lighting_ground(const RC& firesmoke_pos, float fire_light_radius_sq)
  {
    auto* gnd_texture = dynamic_cast<BitmapSprite*>(sprite_ground)->get_curr_sim_frame(get_anim_count(0));
    if (gnd_texture != nullptr)
    {
      auto opaque_pts = sprite_ground->get_opaque_points(get_anim_count(0));
      const auto& sprite_pos = sprite_ground->pos;
      for (const auto& pt : opaque_pts)
      {
        auto r = pt.r - sprite_pos.r;
        auto c = pt.c - sprite_pos.c;
        if (math::distance_squared(pt.r*3.f, static_cast<float>(pt.c), firesmoke_pos.r*3.f, static_cast<float>(firesmoke_pos.c)) < fire_light_radius_sq)
        {
          auto textel = (*gnd_texture)(r, c);
          auto style = textel.get_style();
          auto bright_style = t8::shade_style(style, t8::ShadeType::Bright, true);
          textel.set_style(bright_style);
          gnd_texture->set_textel(r, c, textel);
        }
      }
    }
  }
  
  void update_texts()
  {
    namespace easings = t8x::easings;
  
    float scene_1_time = get_sim_time_s();
    float scene_2_time = get_sim_time_s() - scene_2_start_time;
    if (get_sim_time_s() < scene_2_start_time)
    {
      if (title_anim_0.in_range(scene_1_time))
      {
        auto c_0 = title_anim_0.animate(scene_1_time, 81.f, 8.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        auto c_1 = title_anim_0.animate(scene_1_time, -81.f, 14.f, 81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[0], "Rasmus Anthin", 1, math::roundI(c_0), t8x::Font::SMSlant);
        t8x::draw_text(sh, font_data, color_schemes[0], "wishes you all", 7, math::roundI(c_1), t8x::Font::SMSlant);
      }
      else if (title_anim_1.in_range(scene_1_time))
      {
        auto c = title_anim_1.animate(scene_1_time, 81.f, 9.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[1], "a Very", 3, math::roundI(c), t8x::Font::Larry3D);
      }
      else if (title_anim_2.in_range(scene_1_time))
      {
        auto c_0 = title_anim_2.animate(scene_1_time, 81.f, 7.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        auto c_1 = title_anim_2.animate(scene_1_time, -81.f, 3.f, 81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[1], "Merry", 1, math::roundI(c_0), t8x::Font::Larry3D);
        t8x::draw_text(sh, font_data, color_schemes[1], "Christmas", 8, math::roundI(c_1), t8x::Font::Larry3D);
      }
      else if (title_anim_3.in_range(scene_1_time))
      {
        auto c = title_anim_3.animate(scene_1_time, 81.f, 9.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[1], "and a...", 3, math::roundI(c), t8x::Font::Larry3D);
      }
      else if (title_anim_4.in_range(scene_1_time))
      {
        auto c_0 = title_anim_4.animate(scene_1_time, 81.f, 1.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        auto c_1 = title_anim_4.animate(scene_1_time, -81.f, 9.f, 81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[1], "Happy New", 1, math::roundI(c_0), t8x::Font::Larry3D);
        t8x::draw_text(sh, font_data, color_schemes[1], "Year!", 9, math::roundI(c_1), t8x::Font::Larry3D);
      }
      else if (title_anim_5.in_range(scene_1_time))
      {
        auto c = title_anim_5.animate(scene_1_time, 81.f, 1.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[1], "May 2025", 3, math::roundI(c), t8x::Font::Larry3D);
      }
      else if (title_anim_6.in_range(scene_1_time))
      {
        auto c_0 = title_anim_6.animate(scene_1_time, 81.f, 1.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        auto c_1 = title_anim_6.animate(scene_1_time, -81.f, 7.f, 81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[1], "be an", 1, math::roundI(c_0), t8x::Font::Larry3D);
        t8x::draw_text(sh, font_data, color_schemes[1], "awesome", 8, math::roundI(c_1), t8x::Font::Larry3D);
        t8x::draw_text(sh, font_data, color_schemes[1], "year!", 15, math::roundI(c_0), t8x::Font::Larry3D);
      }
      else if (title_anim_7.in_range(scene_1_time) || !title_anim_8.done(scene_1_time))
      {
        auto t_2 = title_anim_7.animate(scene_1_time, 0.f, 0.5f, 1.f, easings::ease_out_sine, easings::ease_in_sine);
        auto t_3 = title_anim_8.animate(scene_1_time, 0.f, 0.5f, 1.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[0], "GFX via the", f_r(t_2), f_c(t_2), t8x::Font::Avatar);
        t8x::draw_text(sh, font_data, color_schemes[0], "Termin8or lib", 25-f_r(t_3), 7-f_c(t_3), t8x::Font::Avatar);
      }
      else if (title_anim_9.in_range(scene_1_time) || !title_anim_10.done(scene_1_time))
      {
        auto t_4 = title_anim_9.animate(scene_1_time, 0.f, 0.5f, 1.f, easings::ease_out_sine, easings::ease_in_sine);
        auto t_5 = title_anim_10.animate(scene_1_time, 0.f, 0.5f, 1.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[0], "SFX via the", f_r(t_4), f_c(t_4), t8x::Font::Avatar);
        t8x::draw_text(sh, font_data, color_schemes[0], "8Beat lib", 25-f_r(t_5), 7-f_c(t_5), t8x::Font::Avatar);
      }
      else if (title_anim_11.in_range(scene_1_time))
      {
        auto c_0 = title_anim_11.animate(scene_1_time, 81.f, 8.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        auto c_1 = title_anim_11.animate(scene_1_time, -81.f, 14.f, 81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[0], "Made in", 1, math::roundI(c_0), t8x::Font::SMSlant);
        t8x::draw_text(sh, font_data, color_schemes[0], "Sweden!", 7, math::roundI(c_1), t8x::Font::SMSlant);
      }
      else if (title_anim_12.in_range(scene_1_time))
      {
        auto c_0 = title_anim_12.animate(scene_1_time, 81.f, 5.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        auto c_1 = title_anim_12.animate(scene_1_time, -81.f, 7.f, 81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[0], "This demo uses", 1, math::roundI(c_0), t8x::Font::SMSlant);
        t8x::draw_text(sh, font_data, color_schemes[0], "a dynamics system", 7, math::roundI(c_1), t8x::Font::SMSlant);
      }
      else if (title_anim_13.in_range(scene_1_time))
      {
        auto c_0 = title_anim_13.animate(scene_1_time, 81.f, 5.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        auto c_1 = title_anim_13.animate(scene_1_time, -81.f, 7.f, 81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[0], "and collisions", 1, math::roundI(c_0), t8x::Font::SMSlant);
        t8x::draw_text(sh, font_data, color_schemes[0], "for the snow", 7, math::roundI(c_1), t8x::Font::SMSlant);
      }
      else if (title_anim_14.in_range(scene_1_time))
      {
        auto c_0 = title_anim_14.animate(scene_1_time, 81.f, 5.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        auto c_1 = title_anim_14.animate(scene_1_time, -81.f, 7.f, 81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[0], "Lighting effects", 1, math::roundI(c_0), t8x::Font::SMSlant);
        t8x::draw_text(sh, font_data, color_schemes[0], "are done...", 7, math::roundI(c_1), t8x::Font::SMSlant);
      }
      else if (title_anim_15.in_range(scene_1_time))
      {
        auto c_0 = title_anim_15.animate(scene_1_time, 81.f, 5.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        auto c_1 = title_anim_15.animate(scene_1_time, -81.f, 7.f, 81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[0], "by utilizing the", 1, math::roundI(c_0), t8x::Font::SMSlant);
        t8x::draw_text(sh, font_data, color_schemes[0], "distance fields", 7, math::roundI(c_1), t8x::Font::SMSlant);
      }
      else if (title_anim_16.in_range(get_sim_time_s()))
      {
        auto c_0 = title_anim_16.animate(scene_1_time, 81.f, 5.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        auto c_1 = title_anim_16.animate(scene_1_time, -81.f, 7.f, 81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[0], "of the", 1, math::roundI(c_0), t8x::Font::SMSlant);
        t8x::draw_text(sh, font_data, color_schemes[0], "rigidid bodies", 7, math::roundI(c_1), t8x::Font::SMSlant);
      }
      else if (title_anim_17.in_range(scene_1_time))
      {
        auto c_0 = title_anim_17.animate(scene_1_time, 81.f, 5.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        auto c_1 = title_anim_17.animate(scene_1_time, -81.f, 7.f, 81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[0], "that some sprties", 1, math::roundI(c_0), t8x::Font::SMSlant);
        t8x::draw_text(sh, font_data, color_schemes[0], "are attached to", 7, math::roundI(c_1), t8x::Font::SMSlant);
      }
      else if (title_anim_18.in_range(scene_1_time))
      {
        auto c = title_anim_18.animate(scene_1_time, 81.f, -900);
        t8x::draw_text(sh, font_data, color_schemes[0], "Music is not from midi, but via the ChipTuneEngine part of the 8Beat lib which allows you to define waveforms in detail and play these similar to how mods work. The tunes are written in text-based files with the extension: ct.", 1, math::roundI(c), t8x::Font::SMSlant);
      }
      else if (title_anim_19.in_range(scene_1_time))
      {
        auto c = title_anim_19.animate(scene_1_time, 81.f, -500.f);
        t8x::draw_text(sh, font_data, color_schemes[0], "Go ahead and try to modify one of the chiptune files and see what happens!", 1, math::roundI(c), t8x::Font::SMSlant);
      }
      else if (title_anim_20.in_range(scene_1_time))
      {
        auto c_0 = title_anim_20.animate(scene_1_time, 81.f, 8.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        auto c_1 = title_anim_20.animate(scene_1_time, -81.f, 14.f, 81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[0], "Tis the season", 1, math::roundI(c_0), t8x::Font::SMSlant);
        t8x::draw_text(sh, font_data, color_schemes[0], "to be folly", 7, math::roundI(c_1), t8x::Font::SMSlant);
      }
      else if (title_anim_21.in_range(scene_1_time))
      {
        auto c = title_anim_21.animate(scene_1_time, 81.f, 8.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[0], "Falalala la", 1, math::roundI(c), t8x::Font::SMSlant);
      }
      else if (title_anim_22.in_range(scene_1_time))
      {
        auto c = title_anim_22.animate(scene_1_time, -81.f, 14.f, 81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[0], "la la la la", 7, math::roundI(c), t8x::Font::SMSlant);
      }
    }
    else
    {
      if (title_anim_23.in_range(scene_2_time))
      {
        auto c_0 = title_anim_23.animate(scene_2_time, 81.f, 1.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        auto c_1 = title_anim_23.animate(scene_2_time, -81.f, 9.f, 81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[1], "A child was born", 1, math::roundI(c_0), t8x::Font::SMSlant);
        t8x::draw_text(sh, font_data, color_schemes[1], "on Christmas Day", 9, math::roundI(c_1), t8x::Font::SMSlant);
      }
      else if (title_anim_24.in_range(scene_2_time))
      {
        auto c_0 = title_anim_24.animate(scene_2_time, 81.f, 1.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        auto c_1 = title_anim_24.animate(scene_2_time, -81.f, 9.f, 81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[1], "A king that in", 1, math::roundI(c_0), t8x::Font::SMSlant);
        t8x::draw_text(sh, font_data, color_schemes[1], "a manger lay", 9, math::roundI(c_1), t8x::Font::SMSlant);
      }
      else if (title_anim_25.in_range(scene_2_time))
      {
        auto c_0 = title_anim_25.animate(scene_2_time, 81.f, 1.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        auto c_1 = title_anim_25.animate(scene_2_time, -81.f, 9.f, 81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[1], "Jesus Christ", 1, math::roundI(c_0), t8x::Font::SMSlant);
        t8x::draw_text(sh, font_data, color_schemes[1], "a name so bright", 9, math::roundI(c_1), t8x::Font::SMSlant);
      }
      else if (title_anim_26.in_range(scene_2_time))
      {
        auto c_0 = title_anim_26.animate(scene_2_time, 81.f, 1.f, -81.f, easings::ease_out_sine, easings::ease_in_sine);
        auto c_1 = title_anim_26.animate(scene_2_time, -81.f, 9.f, 81.f, easings::ease_out_sine, easings::ease_in_sine);
        t8x::draw_text(sh, font_data, color_schemes[1], "Lit up the world", 1, math::roundI(c_0), t8x::Font::SMSlant);
        t8x::draw_text(sh, font_data, color_schemes[1], "with such might", 9, math::roundI(c_1), t8x::Font::SMSlant);
      }
      else if (title_anim_27.begun(scene_2_time))
      {
        auto c_0 = title_anim_27.animate(scene_2_time, -81.f, 1.f, easings::ease_out_sine);
        auto c_1 = title_anim_27.animate(scene_2_time, -81.f, 9.f, easings::ease_out_sine);
        t8x::draw_text(sh, font_data, color_schemes[1], "The", 1, math::roundI(c_0), t8x::Font::Larry3D);
        t8x::draw_text(sh, font_data, color_schemes[1], "End!", 9, math::roundI(c_1), t8x::Font::Larry3D);
      }
    }
  }
  
  void update_fireplace()
  {
    firesmoke_pos = sprite_fireplace->pos + RC { 0, sprite_fireplace->get_size().c/2 } + RC { 0, fireplace_jitter };
    if (rnd::one_in(3))
      fireplace_jitter = rnd::randn_int(0.f, 0.6f);
    fire_light_radius_sq = math::sq(math::linmap_clamped(get_sim_time_s(), 0.f, 4.f, 0.f, 10.f));
    fire_smoke_engine.update(firesmoke_pos, true, smoke_vel_r, smoke_vel_c, smoke_acc, smoke_spread, smoke_life_time, smoke_cluster_size, get_sim_dt_s(), get_sim_time_s());
    fire_smoke_engine.draw(sh, smoke_color_gradients, get_sim_time_s());
  }
  
  void update_moon()
  {
    moon_angle = m_moon_speed_factor * moon_w * get_sim_time_s() + moon_angle0;
    sprite_moon->pos = t8::to_RC_round({
      moon_pivot.r - 25.f*std::sin(moon_angle),
      moon_pivot.c + 30.f*std::cos(moon_angle)
    });
    
    auto moon_centroid = t8::to_RC_round(sprite_moon->calc_curr_centroid(get_anim_count(0)));
    is_moon_up = math::in_range(std::fmod(math::rad2deg(moon_angle), 360.f), 18.f, 162.f, Range::Open)
      && !sprite_ground->is_opaque(get_anim_count(0), moon_centroid)
      && (!sprite_mountains->is_opaque(get_anim_count(0), moon_centroid)
          || !sprite_mountains->is_opaque(get_anim_count(0), moon_centroid + RC { -2, -1 }) // To reduce flutter.
          || !sprite_mountains->is_opaque(get_anim_count(0), moon_centroid + RC { -2, -2 })); // To reduce flutter.
  }
  
  void update_shadows_and_lighting()
  {
    sprite_ground->fill_sprite_bg_colors(0, is_moon_up ? ground_light_color : ground_dark_color);
    update_lighting_rb_sprite(sprite_mountains, rb_mountains,
                              mountains_dark_style,
                              false, firesmoke_pos, fire_light_radius_sq,
                              is_moon_up, false);
    for (size_t tree_idx = 0; tree_idx < sprite_tree_arr.size(); ++tree_idx)
      update_lighting_rb_sprite(static_cast<BitmapSprite*>(sprite_tree_arr[tree_idx]),
                                rb_tree_arr[tree_idx],
                                tree_dark_style,
                                true, firesmoke_pos, fire_light_radius_sq,
                                is_moon_up, true);
    update_lighting_ground(firesmoke_pos + RC { 1, 0 }, 0.26f*fire_light_radius_sq);
  }
  
  void update_meteors()
  {
    if (get_anim_count(0) > meteor_timestamp)
    {
      sprite_meteor->pos += RC { 1, -1 };
      if (sprite_meteor->pos.r > sh.num_rows())
      {
        sprite_meteor->pos = { -sprite_meteor->get_size().r - 1, sh.num_rows() + rnd::rand_int(0, sh.num_cols()) };
        meteor_timestamp = get_anim_count(0) + rnd::rand_int(10, 120);
      }
    }
  }
  
  void update_critters()
  {
    auto move_critter = [](OneShot& moved_trg, BitmapSprite* critter_sprite, const auto& tree_sprites,
                           int nr, int nc, int gnd_height, int anim_ctr)
    {
      if (moved_trg.once())
      {
        if (rnd::one_in(3))
          critter_sprite->flip_lr();
        
        for (int counter = 0; counter < 1e3_i; ++counter)
        {
          critter_sprite->pos = RC { rnd::rand_int(0, nr - gnd_height), rnd::rand_int(0, nc) };
          auto critter_centroid = t8::to_RC_round(critter_sprite->calc_curr_centroid(anim_ctr));
          for (auto* spr_tree : tree_sprites)
          {
            auto* texture = static_cast<BitmapSprite*>(spr_tree)->get_curr_sim_frame(anim_ctr);
            auto r = critter_centroid.r - spr_tree->pos.r;
            auto c = critter_centroid.c - spr_tree->pos.c;
            if (texture != nullptr)
            {
              auto textel = (*texture)(r, c);
              if (textel.bg_color == Color16::Green || textel.bg_color == Color16::DarkGreen)
                goto critter_placed;
            }
          }
        }
      }
    critter_placed:
      return;
    };
    move_critter(squirrel_moved_trg, sprite_squirrel, sprite_tree_arr,
                 sh.num_rows(), sh.num_cols(), ground_height, get_anim_count(0));
    move_critter(owl_moved_trg, sprite_owl, sprite_tree_arr,
                 sh.num_rows(), sh.num_cols(), ground_height, get_anim_count(0));

  }
  
  float update_wind()
  {
    wind_angle = wind_speed_w * get_sim_time_s() + wind_accumulated_rand_phase;
    auto wind_speed = m_wind_speed_factor * wind_speed_amplitude * std::sin(wind_angle);
    if (rnd::one_in(20))
    {
      wind_accumulated_rand_phase += math::deg2rad(rnd::rand_float(-10.f, 20.f));
      wind_accumulated_rand_phase = std::fmod(wind_accumulated_rand_phase, math::c_2pi);
    }
    if (rnd::one_in(10))
      wind_speed_w = rnd::rand_float(1e-2f, 1e-1f);
    if (rnd::one_in(8))
      wind_speed_amplitude = rnd::rand_float(5.f, 10.f);
    else if(rnd::one_in(3))
      wind_speed_amplitude = rnd::rand_float(0.f, 2.f);
    return wind_speed;
  }
  
  void update_snowflakes(float wind_speed)
  {
    for (auto* rb : rb_snowflake_arr)
    {
      if (rb->get_curr_cm().r >= sh.num_rows())
      {
        rb->reset_curr_cm();
        rb->set_curr_lin_vel(f_snowflake_vel(0));
        snowflake_horiz_vel_factor[rb] = f_calc_per_snowflake_vel_factor();
      }
      
      auto vel_factor = math::value_to_param_clamped(rb->get_curr_cm_r(), m_snow_wind_gradient_max, m_snow_wind_gradient_min);
      auto it_hvf = snowflake_horiz_vel_factor.find(rb);
      auto per_snowflake_vel_factor = 1.f;
      if (it_hvf != snowflake_horiz_vel_factor.end())
        per_snowflake_vel_factor = it_hvf->second;
      rb->set_curr_lin_speed_c(wind_speed * vel_factor * per_snowflake_vel_factor);
      if (rb->get_curr_cm_c() < 0.f)
        rb->set_curr_cm_c(static_cast<float>(sh.num_cols() - 1));
      else if (rb->get_curr_cm_c() >= static_cast<float>(sh.num_cols()))
        rb->set_curr_cm_c(0.f);
        
      if (rb->is_sleeping())
      {
        auto* sprite = dynamic_cast<BitmapSprite*>(rb->get_sprite());
        if (sprite != nullptr)
        {
          auto* texture = sprite->get_curr_sim_frame(get_anim_count(0));
          if (texture != nullptr)
          {
            auto curr_textel = (*texture)(0, 0);
            if (curr_textel.ch != '#')
            {
              curr_textel.ch = '#';
              curr_textel.fg_color = Color16::LightGray;
              curr_textel.bg_color = Color16::LightGray;
              texture->set_textel(0, 0, curr_textel);
              snowflake_map[sprite->pos].emplace_back(sprite);
            }
          }
        }
      }
    }
  }
  
  void update_lake()
  {
    // Lake reflections
    auto* orig_texture = sprite_lake->try_get_frame(1);
    if (orig_texture != nullptr)
      sprite_lake->set_frame(0, *orig_texture);
    auto* lake_texture = sprite_lake->try_get_frame(0);
    if (lake_texture != nullptr)
    {
      offscreen_buffer.buffer_texture = lake_texture;
      sh.print_screen_buffer(Color16::Black, offscreen_buffer);
      for (const auto& rp : reflection_positions)
      {
        lake_texture->set_textel_char(rp.r, rp.c, textel_reflection.ch);
        lake_texture->set_textel_fg_color(rp.r, rp.c, textel_reflection.fg_color);
      }
    }
  }
  
  virtual void on_tune_ended(beat::ChipTuneEngine* engine, const std::string& curr_tune_filepath) override
  {
    std::string tune;
    if (get_sim_time_s() < scene_2_start_time)
    {
      if (curr_tune_filepath.ends_with("deck_the_halls.ct"))
        tune = "silent_night.ct";
      else if (curr_tune_filepath.ends_with("silent_night.ct"))
        tune = "jingle_bells.ct";
      else if (curr_tune_filepath.ends_with("jingle_bells.ct"))
        tune = "deck_the_halls.ct";
      else
        return;
    }
    else
    {
      if (curr_tune_filepath.ends_with("nigh_bethlehem.ct"))
        return;
    }
    
    play_tune(tune);
  }

public:
  Game(int argc, char** argv, const t8x::GameEngineParams& params,
       bool use_audio, float music_volume, bool use_texts,
       float moon_speed_factor, float wind_speed_factor,
       float snow_wind_gradient_max, float snow_wind_gradient_min,
       float fire_wind_speed_factor)
    : GameEngine(argv[0], params)
    , m_fire_wind_speed_factor(fire_wind_speed_factor)
    , m_moon_speed_factor(moon_speed_factor)
    , m_snow_wind_gradient_max(snow_wind_gradient_max)
    , m_snow_wind_gradient_min(snow_wind_gradient_min)
    , m_wind_speed_factor(wind_speed_factor)
    , audio(use_audio)
    , m_music_volume(music_volume)
    , m_use_texts(use_texts)
  {
    //GameEngine::set_real_fps(1000);
    GameEngine::set_anim_rate(0, 4);
    
    chip_tune = std::make_unique<beat::ChipTuneEngine>(audio, wave_gen);
    chip_tune->set_volume_slider(m_music_volume, min_dB, nl_taper);
    chip_tune->add_listener(this);
  }
  
  virtual ~Game() override
  {
    Delay::sleep(100'000);
    chip_tune->remove_listener(this);
    chip_tune.release();
  }
  
  virtual void generate_data() override
  {
    font_data_path = t8x::get_path_to_font_data(get_exe_folder());
    std::cout << font_data_path << std::endl;
    
    t8::Style style_0 { Color16::White, Color16::Red };
    t8::Style style_1 { Color16::Red, Color16::DarkRed };
    t8::Style style_2 { Color16::Red, Color16::White };
    t8::Style style_3 { Color16::Red, Color16::LightGray };
    t8::Style style_4 { Color16::Red, Color16::DarkGray };
    auto& cs0 = color_schemes.emplace_back();
    cs0.internal = style_0;
    cs0.side_h = style_1;
    cs0.side_v = style_1;
    cs0.dot_internal = style_2;
    cs0.dot_side_h = style_3;
    cs0.dot_side_v = style_2;
    auto& cs1 = color_schemes.emplace_back();
    cs1.internal = style_0;
    cs1.side_h = style_1;
    cs1.side_v = style_1;
    cs1.dot_internal = style_2;
    cs1.dot_side_h = style_4;
    cs1.dot_side_v = style_4;
    
    font_data = t8x::load_font_data(font_data_path);
  
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
    
    // Layers:
    // 0 : star
    // 1 : moon
    // 2 : meteor, aurora
    // 3 : ground, mountain
    // 4 : tree, fireplace, lake
    // 5 : snowflake
    // 6 : squirrel, owl
    
  
    sprite_ground = sprh.create_bitmap_sprite("ground");
    sprite_ground->layer_id = 3;
    sprite_ground->pos = { sh.num_rows() - ground_height, 0 };
    sprite_ground->init(ground_height, sh.num_cols());
    sprite_ground->create_frame(0);
    sprite_ground->fill_sprite_chars(0, ' ');
    sprite_ground->fill_sprite_bg_colors(0, ground_dark_color);
    sprite_ground->fill_sprite_materials(0, 1);
    rb_ground = dyn_sys.add_rigid_body(sprite_ground, 0.f, // Zero mass == immovable.
      std::nullopt, {}, {},
      0.f, 0.f,
      e_ground, friction_ground);
      
    sprite_mountains = sprh.create_bitmap_sprite("mountain");
    sprite_mountains->layer_id = 3;
    sprite_mountains->init(13, 70);
    sprite_mountains->pos = { sh.num_rows() - ground_height - sprite_mountains->get_size().r, 0 };
    sprite_mountains->create_frame(0);
    // Forbidden characters: ¨´
    sprite_mountains->set_sprite_chars_from_strings(0,
      R"(/\___.^/""""'.___      /\_.==^=^                                      )", //  0
      R"(      Mm    m   _\__,='       _.\._~.                                 )", //  1
      R"(    ^    ^_    /  \_.    _^. /   \.   \ ,M-.                          )", //  2
      R"(\_ /"'-./  \  /      |  /   \_     \   v   M\                         )", //  3
      R"(  W "" /  wm\/  _.""._\/      \       .      ._                       )", //  4
      R"( v    | w   /  /        \     \   w            \                      )", //  5
      R"(  . :. \   ,   .     m  .   ..::   .      ::..  |                     )", //  6
      R"(       :.  . .   .    '  . ..::...   .   ..:.    \_                   )", //  7
      R"( w.. .    .     .    , . .  :  ::.. .:..:..  ..:   \                  )", //  8
      R"(. ::..,::::... ..  .' ..   . :   ::. .... ...:...  .(o,.              )", //  9
      R"( .    ...  .    . .      . . .     ... :: . ..    .    '.._           )", // 10
      R"(    .   . .    .      .          .              .    .     =-.        )", // 11
      R"( .   - .     -   ~.       ..         ;,.       .       .      }       )"  // 12
    );
    sprite_mountains->fill_sprite_fg_colors(0, Color16::LightGray);
    sprite_mountains->fill_sprite_bg_colors(0, Color16::Transparent2);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 0, 0, 1, Color16::DarkGray); // anim_frame, r, c0, c1, color
    sprite_mountains->fill_sprite_materials_horiz(0, 0, 0, 1, 1);               // anim_frame, r, c0, c1, mat
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 0, 6, 12, Color16::DarkGray);
    sprite_mountains->fill_sprite_materials_horiz(0, 0, 6, 12, 1);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 0, 23, 24, Color16::DarkGray);
    sprite_mountains->fill_sprite_materials_horiz(0, 0, 23, 24, 1);
    sprite_mountains->set_sprite_bg_color(0, 0, 29, Color16::DarkGray); // anim_frame, r, c, color
    sprite_mountains->set_sprite_material(0, 0, 29, 1);
    sprite_mountains->set_sprite_bg_color(0, 0, 31, Color16::DarkGray);
    sprite_mountains->set_sprite_material(0, 0, 31, 1);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 1, 0, 16, Color16::DarkGray);
    sprite_mountains->fill_sprite_materials_horiz(0, 1, 0, 16, 1);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 1, 21, 32, Color16::DarkGray);
    sprite_mountains->fill_sprite_materials_horiz(0, 1, 21, 32, 1);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 2, 0, 39, Color16::DarkGray);
    sprite_mountains->fill_sprite_materials_horiz(0, 2, 0, 39, 1);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 2, 41, 42, Color16::DarkGray);
    sprite_mountains->fill_sprite_materials_horiz(0, 2, 41, 42, 1);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 3, 0, 44, Color16::DarkGray);
    sprite_mountains->fill_sprite_materials_horiz(0, 3, 0, 44, 1);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 4, 0, 45, Color16::DarkGray);
    sprite_mountains->fill_sprite_materials_horiz(0, 4, 0, 45, 1);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 5, 0, 47, Color16::DarkGray);
    sprite_mountains->fill_sprite_materials_horiz(0, 5, 0, 47, 1);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 6, 0, 48, Color16::DarkGray);
    sprite_mountains->fill_sprite_materials_horiz(0, 6, 0, 48, 1);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 7, 0, 49, Color16::DarkGray);
    sprite_mountains->fill_sprite_materials_horiz(0, 7, 0, 49, 1);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 8, 0, 51, Color16::DarkGray);
    sprite_mountains->fill_sprite_materials_horiz(0, 8, 0, 51, 1);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 9, 0, 53, Color16::DarkGray);
    sprite_mountains->fill_sprite_materials_horiz(0, 9, 0, 53, 1);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 10, 0, 55, Color16::DarkGray);
    sprite_mountains->fill_sprite_materials_horiz(0, 10, 0, 55, 1);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 11, 0, 59, Color16::DarkGray);
    sprite_mountains->fill_sprite_materials_horiz(0, 11, 0, 59, 1);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 12, 0, 61, Color16::DarkGray);
    sprite_mountains->fill_sprite_materials_horiz(0, 12, 0, 61, 1);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 0, 8, 12, Color16::DarkGreen);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 0, 27, 28, Color16::DarkGreen);
    sprite_mountains->set_sprite_bg_color(0, 0, 30, Color16::DarkGreen);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 1, 6, 7, Color16::DarkGreen);
    sprite_mountains->set_sprite_bg_color(0, 1, 12, Color16::DarkGreen);
    sprite_mountains->set_sprite_bg_color(0, 3, 4, Color16::DarkGreen);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 4, 4, 5, Color16::DarkGreen);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 4, 10, 11, Color16::DarkGreen);
    sprite_mountains->fill_sprite_bg_colors_horiz(0, 4, 18, 19, Color16::DarkGreen);
    sprite_mountains->set_sprite_bg_color(0, 5, 8, Color16::DarkGreen);
    sprite_mountains->set_sprite_bg_color(0, 5, 34, Color16::DarkGreen);
    sprite_mountains->set_sprite_bg_color(0, 6, 21, Color16::DarkGreen);
    sprite_mountains->set_sprite_bg_color(0, 8, 1, Color16::DarkGreen);
    rb_mountains = dyn_sys.add_rigid_body(sprite_mountains, .0f, std::nullopt);
    
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
      -3, -3, -3, 11, 11, 11, 11, -3, -3, -3,
      -3,  3, 11, 11,  3, 11, 11, 11, 11, -3,
      11, 11,  3, 11,  3, 11, 11,  3, 11, 11,
      -3, 11, 11,  3, 11, 11, 11, 11, 11, -3,
      -3, -3, -3, 11,  3, 11, 11, -3, -3, -3
    );
    sprite_moon->set_sprite_bg_colors(0,
      -3, -3, -3, 11, 11, 11, 11, -3, -3, -3,
      -3, 11, 11, 11, 11, 11, 11, 11, 11, -3,
      11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
      -3, 11, 11, 11, 11, 11, 11, 11, 11, -3,
      -3, -3, -3, 11, 11, 11, 11, -3, -3, -3
    );
    
  
    sprite_tree = sprh.create_bitmap_sprite("tree");
    sprite_tree->layer_id = 4;
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
      -1, -1, -1, -1, 10, -1, -1, -1, -1,
      -1, -1, -1, 10, 10, 10, -1, -1, -1,
      -1, -1, 10, 10, 10, 10, 10, -1, -1,
      -1, -1, 10, 10, 10, 10, 10, -1, -1,
      -1, 10, 10, 10, 10, 10, 10, 10, -1,
      -1, 10, 10, 10, 10, 10, 10, 10, -1,
      10, 10, 10, 10, 10, 10, 10, 10, 10,
      10, 10, 10, -1,  9, -1, 10, 10, 10,
      -1, -1, -1, -1,  9, -1, -1, -1, -1,
      -1, -1, -1, -1,  9, -1, -1, -1, -1
    );
    sprite_tree->set_sprite_bg_colors(0,
      -3, -3, -3, -3,  2, -3, -3, -3, -3,
      -3, -3, -3,  2,  2,  2, -3, -3, -3,
      -3, -3,  2,  2,  2,  2,  2, -3, -3,
      -3, -3,  2,  2,  2,  2,  2, -3, -3,
      -3,  2,  2,  2,  2,  2,  2,  2, -3,
      -3,  2,  2,  2,  2,  2,  2,  2, -3,
       2,  2,  2,  2,  2,  2,  2,  2,  2,
       2,  2,  2, -3,  1, -3,  2,  2,  2,
      -3, -3, -3, -3,  1, -3, -3, -3, -3,
      -3, -3, -3, -3,  1, -3, -3, -3, -3
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
    sprite_tree_arr = sprh.clone_sprite_array<10>("tree", "tree");
    sprite_tree->enabled = false;
    rb_tree_arr = dyn_sys.add_rigid_bodies<10>(sprite_tree_arr,
      [](int){ return 0.f; }, // mass
      [](int){ return std::nullopt; }, // pos, nullopt because static rigid objects.
      [](int){ return Vec2 {}; }, // vel
      [](int){ return Vec2 {}; }, // force
      [](int){ return 0.f; }, // ang_vel
      [](int){ return 0.f; }, // torque
      [this](int){ return e_tree; },
      [this](int){ return friction_tree; },
      [](int){ return 0.f; }, // crit speed r
      [](int){ return 0.f; }, // crit speed c
      [](int){ return std::vector { 1 }; }, // inertia mats
      [](int){ return std::vector { 1 }; } // coll mats
    );
    for (auto* sprite : sprite_tree_arr)
    {
      auto* bmp_sprite = static_cast<BitmapSprite*>(sprite);
      bmp_sprite->pos.r = sh.num_rows() - sprite_tree->get_size().r - ground_height + rnd::rand_int(0, 1);
      bmp_sprite->pos.c = rnd::rand_int(0, 80);
      if (rnd::one_in(3))
        bmp_sprite->flip_lr(0);
    }
    
    sprite_fireplace = sprh.create_bitmap_sprite("fireplace");
    sprite_fireplace->layer_id = 4;
    sprite_fireplace->init(2, 8);
    sprite_fireplace->pos = { sh.num_rows() - ground_height + 1, 35 };
    sprite_fireplace->create_frame(0);
    sprite_fireplace->set_sprite_chars_from_strings(0,
      R"( //Oo\\ )",
      R"(/()==\O))"
    );
    sprite_fireplace->set_sprite_fg_colors(0,
      Color16::Transparent2, Color16::DarkRed, Color16::Red, Color16::DarkRed, Color16::DarkRed, Color16::Red, Color16::DarkRed, Color16::Transparent2,
      Color16::DarkRed, Color16::Red, Color16::Red, Color16::Red, Color16::Red, Color16::Red, Color16::Red, Color16::DarkRed
    );
    sprite_fireplace->set_sprite_bg_colors(0,
      Color16::Transparent2, Color16::Transparent2, Color16::DarkRed, Color16::Black, Color16::Black, Color16::DarkRed, Color16::Transparent2, Color16::Transparent2,
      Color16::Transparent2, Color16::DarkRed, Color16::DarkRed, Color16::DarkRed, Color16::DarkRed, Color16::DarkRed, Color16::DarkRed, Color16::Transparent2
    );
    sprite_fireplace->clone_frame(1, 0);
    sprite_fireplace->set_sprite_bg_color(1, 0, 3, Color16::Red);
    sprite_fireplace->clone_frame(2, 0);
    sprite_fireplace->set_sprite_bg_color(2, 0, 4, Color16::Red);
    sprite_fireplace->func_calc_anim_frame = [](int sim_frame)
    {
      int anim_frame = sim_frame % 10;
      switch (anim_frame)
      {
        case 0: return 0;
        case 1: return 1;
        case 2: return 0;
        case 3: return 2;
        case 4: return 0;
        case 5: return 0;
        default: return rnd::rand_int(0, 2);
      }
      return 0;
    };
    
    smoke_color_gradients.emplace_back(0.5f, smoke_0);
    smoke_color_gradients.emplace_back(0.6f, smoke_1);
    
    sprite_squirrel = sprh.create_bitmap_sprite("squirrel");
    sprite_squirrel->layer_id = 6;
    sprite_squirrel->init(2, 3);
    sprite_squirrel->create_frame(0);
    sprite_squirrel->set_sprite_chars_from_strings(0,
      R"(^^S)",
      R"(oo/)"
    );
    sprite_squirrel->set_sprite_fg_colors(0,
      Color16::White, Color16::White, Color16::DarkRed,
      Color16::Black, Color16::Black, Color16::Red
    );
    sprite_squirrel->set_sprite_bg_colors(0,
      Color16::Red, Color16::Red, Color16::Red,
      Color16::Red, Color16::Red, Color16::DarkRed
    );
    sprite_squirrel->create_frame(1);
    sprite_squirrel->set_sprite_chars_from_strings(1,
      R"(^^S)",
      R"(  /)"
    );
    sprite_squirrel->set_sprite_fg_colors(1,
      Color16::White, Color16::White, Color16::DarkRed,
      Color16::Transparent2, Color16::Transparent2, Color16::Red
    );
    sprite_squirrel->set_sprite_bg_colors(1,
      Color16::Red, Color16::Red, Color16::Red,
        Color16::Transparent2, Color16::Transparent2, Color16::DarkRed
    );
    sprite_squirrel->create_frame(2);
    sprite_squirrel->fill_sprite_chars(2, ' ');
    sprite_squirrel->fill_sprite_fg_colors(2, Color16::Transparent2);
    sprite_squirrel->fill_sprite_bg_colors(2, Color16::Transparent2);
    sprite_squirrel->enabled = true;
    sprite_squirrel->func_calc_anim_frame = [this](int sim_frame)
    {
      if (squirrel_timestamp <= sim_frame)
      {
        auto diff =  sim_frame - squirrel_timestamp;
        int anim_frame = diff % 18;
        if (squirrel_mode == 0)
        {
          switch (anim_frame)
          {
            case 0:
            case 1:
              return 1; // partial
            case 2:
            case 3:
            case 4:
              return 0; // full
            case 5:
            case 6:
              return 1; // partial
            case 7:
              squirrel_timestamp = sim_frame + rnd::rand_int(0, 200);
              squirrel_mode = rnd::rand_int(0, 1);
              squirrel_moved_trg.reset();
              return 2; // gone
          }
        }
        else
        {
          switch (anim_frame)
          {
            case 0:
            case 1:
              return 1; // partial
            case 2:
            case 3:
            case 4:
              return 0; // full
            case 5:
            case 6:
            case 7:
              return 1; // partial
            case 8:
            case 9:
              return 2; // gone
            case 10:
            case 11:
              return 1; // partial
            case 12:
            case 13:
            case 14:
              return 0; // full
            case 15:
            case 16:
              return 1; // partial
            case 17:
              squirrel_timestamp = sim_frame + rnd::rand_int(0, 200);
              squirrel_mode = rnd::rand_int(0, 1);
              squirrel_moved_trg.reset();
              return 2; // gone
          }
        }
      }
      return 2; // gone
    };
    
    sprite_owl = sprh.create_bitmap_sprite("owl");
    sprite_owl->layer_id = 6;
    sprite_owl->init(3, 2);
    sprite_owl->create_frame(0);
    sprite_owl->set_sprite_chars_from_strings(0,
      R"(oo)",
      R"((\)",
      R"(mm)"
    );
    sprite_owl->set_sprite_fg_colors(0,
      Color16::Black, Color16::Black,
      Color16::DarkGray, Color16::LightGray,
      Color16::DarkGray, Color16::DarkGray
    );
    sprite_owl->set_sprite_bg_colors(0,
      Color16::LightGray, Color16::LightGray,
      Color16::LightGray, Color16::DarkGray,
      Color16::Transparent2, Color16::Transparent2
    );
    sprite_owl->create_frame(1);
    sprite_owl->set_sprite_chars_from_strings(1,
      R"(--)",
      R"((\)",
      R"(mm)"
    );
    sprite_owl->set_sprite_fg_colors(1,
      Color16::Black, Color16::Black,
      Color16::DarkGray, Color16::LightGray,
      Color16::DarkGray, Color16::DarkGray
    );
    sprite_owl->set_sprite_bg_colors(1,
      Color16::LightGray, Color16::LightGray,
      Color16::LightGray, Color16::DarkGray,
      Color16::Transparent2, Color16::Transparent2
    );
    sprite_owl->create_frame(2);
    sprite_owl->set_sprite_chars_from_strings(2,
      R"(oo)",
      R"(( )",
      R"(  )"
    );
    sprite_owl->set_sprite_fg_colors(2,
      Color16::Black, Color16::Black,
      Color16::DarkGray, Color16::Transparent2,
      Color16::Transparent2, Color16::Transparent2
    );
    sprite_owl->set_sprite_bg_colors(2,
      Color16::LightGray, Color16::LightGray,
      Color16::LightGray, Color16::Transparent2,
      Color16::Transparent2, Color16::Transparent2
    );
    sprite_owl->create_frame(3);
    sprite_owl->fill_sprite_chars(3, ' ');
    sprite_owl->fill_sprite_fg_colors(3, Color16::Transparent2);
    sprite_owl->fill_sprite_bg_colors(3, Color16::Transparent2);
    sprite_owl->enabled = true;
    sprite_owl->func_calc_anim_frame = [this](int sim_frame)
    {
      if (owl_timestamp <= sim_frame)
      {
        auto diff =  sim_frame - owl_timestamp;
        int anim_frame = diff % 17;
        switch (anim_frame)
        {
          case 0:
          case 1:
            return 2; // partial
          case 2:
          case 3:
          case 4:
          case 5:
            return 0; // full
          case 6:
          case 7:
            return 1; // blink
          case 8:
          case 9:
          case 10:
            return 0; // full
          case 11:
            return 1; // blink
          case 12:
          case 13:
          case 14:
          case 15:
            return 2; // partial
          case 16:
            owl_timestamp = sim_frame + rnd::rand_int(0, 200);
            owl_moved_trg.reset();
            return 3; // gone
        }
      }
      return 3; // gone
    };
    
    sprite_lake = sprh.create_bitmap_sprite("lake");
    sprite_lake->pos = sprite_fireplace->pos + RC { 0, 11 };
    sprite_lake->layer_id = 4;
    sprite_lake->init(3, 35);
    sprite_lake->create_frame(0);
    sprite_lake->fill_sprite_chars(0, ' ');
    sprite_lake->fill_sprite_fg_colors(0, Color16::Black);
    // upside down.
    sprite_lake->set_sprite_bg_colors(0,
      -3, -3, -3,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -3, -3, -3, -3, -3,
       0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,
      -3, -3, -3, -3, -3, -3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0, -3, -3
    );
    sprite_lake->clone_frame(1, 0);
    
    offscreen_buffer.buffer_screen_pos = sprite_lake->pos + RC { -9, 0 };
    offscreen_buffer.exclude_src_chars = { ' ' };
    offscreen_buffer.exclude_src_bg_colors = { Color16::DarkGray };
    offscreen_buffer.dst_fill_bg_colors = { Color16::Black };
    offscreen_buffer.replace_src_dst_bg_colors.emplace_back(Color16::Transparent2, Color16::Black);
    
    sprite_meteor = sprh.create_bitmap_sprite("meteor");
    sprite_meteor->pos.r = sh.num_rows();
    sprite_meteor->layer_id = 2;
    sprite_meteor->init(4, 5);
    sprite_meteor->create_frame(0);
    sprite_meteor->set_sprite_chars_from_strings(0,
      "    /",
      "   / ",
      " //  ",
      " O   "
    );
    sprite_meteor->set_sprite_fg_colors(0,
      Color16::Transparent2, Color16::Transparent2, Color16::Transparent2, Color16::Transparent2, Color16::Yellow,
      Color16::Transparent2, Color16::Transparent2, Color16::Transparent2, Color16::Yellow, Color16::Transparent2,
      Color16::Transparent2, Color16::Yellow, Color16::Yellow, Color16::Transparent2, Color16::Transparent2,
      Color16::Transparent2, Color16::Red, Color16::Transparent2, Color16::Transparent2, Color16::Transparent2
    );
    sprite_meteor->fill_sprite_bg_colors(0, Color16::Transparent2);
    
    sprite_snowflake = sprh.create_bitmap_sprite("snowflake");
    sprite_snowflake->layer_id = 5;
    sprite_snowflake->pos = { 0, 27 };
    sprite_snowflake->init(1, 1);
    sprite_snowflake->create_frame(0);
    sprite_snowflake->set_sprite_chars(0, '*');
    sprite_snowflake->set_sprite_fg_colors(0, Color16::White);
    sprite_snowflake->set_sprite_bg_colors(0, Color16::Transparent2);
    sprite_snowflake->set_sprite_materials(0, 1);
    f_snowflake_vel = [](int) -> Vec2 { return Vec2 { rnd::rand_float(0.4f, 0.6f), rnd::rand_float(-4.f, -2.f)}; };
    sprite_snowflake_arr = sprh.clone_sprite_array<1000>("snowflake", "snowflake");
    sprite_snowflake->enabled = false;
    rb_snowflake_arr = dyn_sys.add_rigid_bodies<1000>(sprite_snowflake_arr,
      [](int) { return 0.5f; }, // mass
      [](int) { return Vec2 { rnd::rand_float(-800.f, 0.f), rnd::rand_float(-10.f, 70.f) }; }, // pos
      f_snowflake_vel, // vel
      [](int) { return Vec2 { 0.1f, 0.f }; }, // force
      [](int) { return 0.f; }, // ang_vel
      [](int) { return 0.f; }, // torque
      [this](int) { return e_snowflake; },
      [this](int) { return friction_snowflake; },
      [](int){ return rnd::rand_float(0.7f, 4.f); } // crit speed r
    );
    for (auto* rb : rb_snowflake_arr)
    {
      rb->set_sleeping(true,
                       0.05f, 0.5f, // vel, time
                       10.f); // impulse
      snowflake_horiz_vel_factor[rb] = f_calc_per_snowflake_vel_factor();
    }
    
    
    generate_star_sprites();
    
    sprite_aurora = sprh.create_bitmap_sprite("aurora borealis");
    sprite_aurora->layer_id = 1;
    sprite_aurora->init(5, 15);
    sprite_aurora->create_frame(0);
    sprite_aurora->set_sprite_chars_from_strings(0,
      "               ",
      "               ",
      "               ",
      "               ",
      "               "
    );
    sprite_aurora->fill_sprite_fg_colors(0, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(0, Color16::Transparent2);
    sprite_aurora->create_frame(1);
    sprite_aurora->set_sprite_chars_from_strings(1,
      "       ..      ",
      "      .    .   ",
      "               ",
      "  .     .    . ",
      "               "
    );
    sprite_aurora->fill_sprite_fg_colors(1, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(1, Color16::Transparent2);
    sprite_aurora->create_frame(2);
    sprite_aurora->set_sprite_chars_from_strings(2,
      "       ./      ",
      "     /.    .   ",
      "    .          ",
      "  .     .    . ",
      "               "
    );
    sprite_aurora->fill_sprite_fg_colors(2, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(2, Color16::Transparent2);
    sprite_aurora->create_frame(3);
    sprite_aurora->set_sprite_chars_from_strings(3,
      "       ./      ",
      "     /.    .   ",
      "    .  /       ",
      "  .     .    . ",
      "               "
    );
    sprite_aurora->fill_sprite_fg_colors(3, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(3, Color16::Transparent2);
    sprite_aurora->create_frame(4);
    sprite_aurora->set_sprite_chars_from_strings(4,
      "       ./      ",
      "     /.    ./  ",
      "    .  /       ",
      "  .     .    . ",
      "               "
    );
    sprite_aurora->fill_sprite_fg_colors(4, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(4, Color16::Transparent2);
    sprite_aurora->create_frame(5);
    sprite_aurora->set_sprite_chars_from_strings(5,
      "       ./      ",
      "     /.    ./  ",
      "    .  /       ",
      "  .     ./   . ",
      "               "
    );
    sprite_aurora->fill_sprite_fg_colors(5, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(5, Color16::Transparent2);
    sprite_aurora->create_frame(6);
    sprite_aurora->set_sprite_chars_from_strings(6,
      "       ./      ",
      "     /.    ./  ",
      "    .  /       ",
      "  ./    ./   . ",
      "           .   "
    );
    sprite_aurora->fill_sprite_fg_colors(6, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(6, Color16::Transparent2);
    sprite_aurora->create_frame(7);
    sprite_aurora->set_sprite_chars_from_strings(7,
      "       ./      ",
      "     /.    ./  ",
      "    /.  /      ",
      "  ./    ./   . ",
      "            ,  "
    );
    sprite_aurora->fill_sprite_fg_colors(7, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(7, Color16::Transparent2);
    sprite_aurora->create_frame(8);
    sprite_aurora->set_sprite_chars_from_strings(8,
      "       ./      ",
      "     //    ./  ",
      "    /.  /      ",
      "  ./    ./   . ",
      "      .    ,   "
    );
    sprite_aurora->fill_sprite_fg_colors(8, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(8, Color16::Transparent2);
    sprite_aurora->create_frame(9);
    sprite_aurora->set_sprite_chars_from_strings(9,
      "       .//     ",
      "     //    ./  ",
      "    /.  /      ",
      "  ./    ./   . ",
      "      .    ,   "
    );
    sprite_aurora->fill_sprite_fg_colors(9, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(9, Color16::Transparent2);
    sprite_aurora->create_frame(10);
    sprite_aurora->set_sprite_chars_from_strings(10,
      "       .// /   ",
      "     //    ./  ",
      "     /./ /     ",
      "  ./    ./   . ",
      "      .   ,    "
    );
    sprite_aurora->fill_sprite_fg_colors(10, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(10, Color16::Transparent2);
    sprite_aurora->create_frame(11);
    sprite_aurora->set_sprite_chars_from_strings(11,
      "       .// / / ",
      "    //   ./    ",
      "    //./ /     ",
      "   ./ /  ./  . ",
      "     /.   ,    "
    );
    sprite_aurora->fill_sprite_fg_colors(11, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(11, Color16::Transparent2);
    sprite_aurora->create_frame(12);
    sprite_aurora->set_sprite_chars_from_strings(12,
      "      .:// / / ",
      "   ,//  ./     ",
      "    //./ //.   ",
      "    .//   .//. ",
      "    /:. / ,    "
    );
    sprite_aurora->fill_sprite_fg_colors(12, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(12, Color16::Transparent2);
    sprite_aurora->create_frame(13);
    sprite_aurora->set_sprite_chars_from_strings(13,
      "     .:// / // ",
      "  /,// :/      ",
      "    //./ //.:  ",
      "     .//, .//. ",
      "    /:.// ,    "
    );
    sprite_aurora->fill_sprite_fg_colors(13, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(13, Color16::Transparent2);
    sprite_aurora->create_frame(14);
    sprite_aurora->set_sprite_chars_from_strings(14,
      "    .:/// / //.",
      " /,//:/        ",
      "   .//.////.:  ",
      "      .//, .//.",
      "    /:.// ,  / "
    );
    sprite_aurora->fill_sprite_fg_colors(14, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(14, Color16::Transparent2);
    sprite_aurora->create_frame(15);
    sprite_aurora->set_sprite_chars_from_strings(15,
      "   ,.:/// ////,",
      "// /:/         ",
      "  . .//.////.:.",
      "       .//,.//.",
      "   / /:.// , / "
    );
    sprite_aurora->fill_sprite_fg_colors(15, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(15, Color16::Transparent2);
    sprite_aurora->create_frame(16);
    sprite_aurora->set_sprite_chars_from_strings(16,
      " . ,.:/// ////,",
      "///:/          ",
      " .. ://./////..",
      "        .//,//.",
      " / / /:.// ,/  "
    );
    sprite_aurora->fill_sprite_fg_colors(16, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(16, Color16::Transparent2);
    sprite_aurora->create_frame(17);
    sprite_aurora->set_sprite_chars_from_strings(17,
      " . ,./////.///,",
      "////           ",
      " ..::////////..",
      "         .////.",
      " / ///.// /,/  "
    );
    sprite_aurora->fill_sprite_fg_colors(17, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(17, Color16::Transparent2);
    sprite_aurora->create_frame(18);
    sprite_aurora->set_sprite_chars_from_strings(18,
      " .,//////.///, ",
      " // //.        ",
      " ../:////:///..",
      "        .// //.",
      "   ////.///,/  "
    );
    sprite_aurora->fill_sprite_fg_colors(18, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(18, Color16::Transparent2);
    sprite_aurora->create_frame(19);
    sprite_aurora->set_sprite_chars_from_strings(19,
      " .,//////.///, ",
      "  /,// /.      ",
      "../:////:// /..",
      "       .//,//. ",
      "  / ////.///,/ "
    );
    sprite_aurora->fill_sprite_fg_colors(19, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(19, Color16::Transparent2);
    sprite_aurora->create_frame(20);
    sprite_aurora->set_sprite_chars_from_strings(20,
      " .,//////.///,.",
      "   /,/ / /.    ",
      "../:////:// /..",
      "     ./ /,// . ",
      " //////.///,/  "
    );
    sprite_aurora->fill_sprite_fg_colors(20, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(20, Color16::Transparent2);
    sprite_aurora->create_frame(21);
    sprite_aurora->set_sprite_chars_from_strings(21,
      " .,//////.///,.",
      "   / ,/ / /.   ",
      "../:////:// /..",
      "    ./ /,/ / . ",
      " // ////.///,/ "
    );
    sprite_aurora->fill_sprite_fg_colors(21, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(21, Color16::Transparent2);
    sprite_aurora->create_frame(22);
    sprite_aurora->set_sprite_chars_from_strings(22,
      "/.,////././//,.",
      "     /,/ / /.  ",
      "./////:/ / /:. ",
      "  ./ /,/ /..   ",
      " //// //.///,/ "
    );
    sprite_aurora->fill_sprite_fg_colors(22, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(22, Color16::Transparent2);
    sprite_aurora->create_frame(23);
    sprite_aurora->set_sprite_chars_from_strings(23,
      "/.,////././//,.",
      "       /,// /. ",
      "./////// ///:/.",
      " .//,/ /..     ",
      " // /// /.//// "
    );
    sprite_aurora->fill_sprite_fg_colors(23, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(23, Color16::Transparent2);
    sprite_aurora->create_frame(24);
    sprite_aurora->set_sprite_chars_from_strings(24,
      " /,// /././ //,",
      "        /,///. ",
      ". //// // ///:/",
      "./// /. .      ",
      " / / /// /./// "
    );
    sprite_aurora->fill_sprite_fg_colors(24, Color16::Green);
    sprite_aurora->fill_sprite_bg_colors(24, Color16::Transparent2);
    //sprite_aurora->fill_sprite_bg_colors_horiz(0, 0, 7, 8, Color16::Cyan);
    //sprite_aurora->set_sprite_bg_colors(0, 1, 6, Color16::Cyan);
    
    sprite_aurora->func_calc_anim_frame = [this](int sim_frame)
    {
      if (aurora_timestamp <= sim_frame)
      {
        auto diff =  sim_frame - aurora_timestamp;
        int anim_frame = diff % 50;
        if (0 <= anim_frame && anim_frame < 24)
          return anim_frame;
        else if (24 <= anim_frame && anim_frame <= 48)
          return 48 - anim_frame;
        else
        {
          aurora_timestamp = sim_frame + rnd::rand_int(0, 200);
          sprite_aurora->pos = { rnd::rand_int(0, sh.num_rows() - 1), rnd::rand_int(0, sh.num_cols() - 1) };
          return 0;
        }
      }
      return 0;
    };
    
    coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "tree", "tree");
    coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "tree", "ground");
    coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "snow", "snow"); // Short prefixes to make it a bit faster.
    coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "snowflake", "ground");
    coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "mount", "tree");
    coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "mount", "snow");
    coll_handler.rebuild_BVH(sh.num_rows(), sh.num_cols(), &dyn_sys);
  }
  
private:

  SpriteHandler sprh;
  DynamicsSystem dyn_sys;
  CollisionHandler coll_handler;
  
  const int ground_height = 5;
  BitmapSprite* sprite_ground = nullptr;
  RigidBody* rb_ground = nullptr;
  Color ground_dark_color = Color16::DarkGray;
  Color ground_light_color = Color16::LightGray;
  Color ground_shadow_color = Color16::DarkGray;
  
  BitmapSprite* sprite_tree = nullptr;
  std::array<Sprite*, 10> sprite_tree_arr;
  std::array<RigidBody*, 10> rb_tree_arr;
  t8::Style tree_dark_style { Color16::Green, Color16::DarkGreen };

  BitmapSprite* sprite_moon = nullptr;
  
  BitmapSprite* sprite_fireplace = nullptr;
  int fireplace_jitter = 0;
  RC firesmoke_pos;
  float fire_light_radius_sq = 0.f;
  
  BitmapSprite* sprite_mountains = nullptr;
  RigidBody* rb_mountains = nullptr;
  t8::Style mountains_dark_style { Color16::LightGray, Color16::DarkGray };
    
  BitmapSprite* sprite_snowflake = nullptr;
  std::array<Sprite*, 1000> sprite_snowflake_arr;
  std::array<RigidBody*, 1000> rb_snowflake_arr;
  std::map<RC, std::vector<Sprite*>> snowflake_map;
  std::function<Vec2(int)> f_snowflake_vel;
  std::map<RigidBody*, float> snowflake_horiz_vel_factor;
  std::function<float()> f_calc_per_snowflake_vel_factor = []() { return rnd::randn_range_clamp(0.1f, 2.f); };
  
  BitmapSprite* sprite_squirrel = nullptr;
  int squirrel_timestamp = 50;
  int squirrel_mode = 0;
  OneShot squirrel_moved_trg;
  
  BitmapSprite* sprite_owl = nullptr;
  int owl_timestamp = 70;
  OneShot owl_moved_trg;
  
  BitmapSprite* sprite_lake = nullptr;
  t8::OffscreenBuffer offscreen_buffer;
  t8::Textel textel_reflection { '/', Color16::Blue, Color16::Transparent2 };
  std::vector<RC> reflection_positions { { 1, 10 }, { 1, 11 }, { 1, 24 }, { 1, 26 }, { 1, 28 }, { 1, 29 } };
  
  BitmapSprite* sprite_meteor = nullptr;
  int meteor_timestamp = 20;
  
  BitmapSprite* sprite_aurora = nullptr;
  int aurora_timestamp = 82;
  
  BitmapSprite* sprite_manger = nullptr;
  
  BitmapSprite* sprite_bethlehem_star = nullptr;
  
  BitmapSprite* sprite_palm_tree = nullptr;
  std::array<Sprite*, 3> sprite_palm_tree_arr;
  std::array<int, 3> palm_tree_anim_offs { 2, 3, 6 };
  
  BitmapSprite* sprite_camel = nullptr;
  TransitionAnimationSingle anim_camel { 12.f,  0.f, 8.f };
  
  BitmapSprite* sprite_maria = nullptr;
  
  BitmapSprite* sprite_josef = nullptr;
  TransitionAnimationSingle anim_josef { 2.f, 0.f, 8.f };
  
  BitmapSprite* sprite_house = nullptr;
  
  BitmapSprite* sprite_shepherd = nullptr;
  std::array<Sprite*, 3> sprite_shepherd_arr;
  std::array<TransitionAnimationSingle, 3> anim_shepherd_arr
  {
    TransitionAnimationSingle { 22.f, 0.f, 8.f },
    TransitionAnimationSingle { 30.f, 0.f, 8.f },
    TransitionAnimationSingle { 38.f, 0.f, 8.f }
  };
  
  BitmapSprite* sprite_lamb = nullptr;
  std::array<Sprite*, 3> sprite_lamb_arr;
  std::array<TransitionAnimationSingle, 3> anim_lamb_arr
  {
    TransitionAnimationSingle { 24.f, 0.f, 6.f },
    TransitionAnimationSingle { 34.f, 0.f, 6.f },
    TransitionAnimationSingle { 39.f, 0.f, 6.f }
  };
  
  ParticleHandler fire_smoke_engine { 500 };
  
  ParticleGradientGroup smoke_0
  {
    {
      {
        { 0.00f, Color16::White },
        { 0.27f, Color16::Red },
        { 0.60f, Color16::Yellow },
        { 0.75f, Color16::LightGray },
        { 0.88f, Color16::DarkGray },
      }
    },
    {
      {
        { 0.12f, Color16::Blue },
        { 0.20f, Color16::White },
        { 0.30f, Color16::Yellow },   // 0.125f
        { 0.55f, Color16::DarkRed },  // 0.375f
        { 0.70f, Color16::DarkGray }, // 0.625f
        { 0.85f, Color16::Black },    // 0.875f
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
        { 0.10f, Color16::White },
        { 0.37f, Color16::Red },
        { 0.70f, Color16::Yellow },
        { 0.85f, Color16::LightGray },
        { 0.98f, Color16::DarkGray },
      }
    },
    {
      {
        { 0.05f, Color16::Blue },
        { 0.15f, Color16::White },
        { 0.50f, Color16::Yellow },   // 0.125f
        { 0.74f, Color16::DarkRed },  // 0.375f
        { 0.86f, Color16::DarkGray }, // 0.625f
        { 1.00f, Color16::Black },    // 0.875f
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
  
  const float smoke_life_time = 1.2f;
  const float smoke_spread = 3.f;
  const int smoke_cluster_size = 10;
  const float smoke_vel_r = -2.5f;
  float smoke_vel_c = 0.1f;
  const float smoke_acc = -1.5f;
  float m_fire_wind_speed_factor = 1.f;
  
  const Vec2 moon_pivot = { 30.f, 37.f };
  const float moon_w = 8e-4f * math::c_2pi; //5e-4f * math::c_2pi;
  const float moon_angle0 = math::deg2rad(7.5f);
  float moon_angle = 0.f;
  bool is_moon_up = false;
  float m_moon_speed_factor = 1.f;
  
  float e_ground = 0.05f;
  float e_tree = 0.1f;
  float e_snowflake = 0.07f;
  float friction_ground = 0.5f;
  float friction_tree = 0.95f;
  float friction_snowflake = 0.8f;
  float m_snow_wind_gradient_max = 20.f;
  float m_snow_wind_gradient_min = 0.f;
  
  float wind_speed_amplitude = 5.f;
  float wind_speed_w = 1e-1f * math::c_2pi;
  float wind_accumulated_rand_phase = math::deg2rad(rnd::rand_float(0.f, 45.f));
  float wind_angle = 0.f;
  float m_wind_speed_factor = 1.f;
  
  beat::AudioSourceHandler audio;
  float m_music_volume = c_default_vol;
  beat::WaveformGeneration wave_gen;
  std::unique_ptr<beat::ChipTuneEngine> chip_tune;
  OneShot trg_scene_2_tune;
  float min_dB = -60.f;
  float nl_taper = 0.28f;
  
  std::vector<t8x::ColorScheme> color_schemes;
  std::string font_data_path;
  t8x::FontDataColl font_data;
  
  TransitionAnimationInOut title_anim_0 { 0.f,  0.f, 2.f, 6., 8.f }; // Rasmus Anthin / wishes you all
  TransitionAnimationInOut title_anim_1 { 12.f, 0.f, 2.f, 8.f, 10.f }; // a Very
  TransitionAnimationInOut title_anim_2 { 24.f, 0.f, 2.f, 8.f, 10.f }; // Merry / Christmas
  TransitionAnimationInOut title_anim_3 { 38.f, 0.f, 2.f, 4.f, 5.f }; // and a...
  TransitionAnimationInOut title_anim_4 { 45.f, 0.f, 2.f, 4.f, 5.f }; // Happy New / Year!
  TransitionAnimationInOut title_anim_5 { 52.f, 0.f, 2.f, 4.f, 5.f }; // May 2025
  TransitionAnimationInOut title_anim_6 { 59.f, 0.f, 2.f, 4.f, 5.f }; // Be an awesome / year!
  
  TransitionAnimationInOut title_anim_7 { 73.f, 0.f, 2.f, 4.f, 5.f }; // Graphics via the...
  TransitionAnimationInOut title_anim_8 { 74.f, 0.f, 2.f, 4.f, 5.f }; // Termin8or library
  TransitionAnimationInOut title_anim_9 { 81.f, 0.f, 2.f, 4.f, 5.f }; // Sound via the...
  TransitionAnimationInOut title_anim_10 { 82.f, 0.f, 2.f, 4.f, 5.f }; // 8Beat library
  TransitionAnimationInOut title_anim_11 { 87.f, 0.f, 2.f, 5.f, 7.f }; // Made in / Sweden!
  
  TransitionAnimationInOut title_anim_12 { 99.f, 0.f, 2.f, 5.f, 7.f }; // This demo uses / a dynamics system
  TransitionAnimationInOut title_anim_13 { 111.f, 0.f, 2.f, 5.f, 7.f }; // and collisions / for the snow
  TransitionAnimationInOut title_anim_14 { 120.f, 0.f, 2.f, 5.f, 7.f }; // Lighting effects / are done...
  TransitionAnimationInOut title_anim_15 { 130.f, 0.f, 2.f, 5.f, 7.f }; // by utilizing the / distance fields
  TransitionAnimationInOut title_anim_16 { 140.f, 0.f, 2.f, 5.f, 7.f }; // of the / rigidid bodies
  TransitionAnimationInOut title_anim_17 { 150.f, 0.f, 2.f, 5.f, 7.f }; // that some sprties / are attached to
  
  TransitionAnimationSingle title_anim_18 { 162.f, 0.f, 50.f }; // Music is not from midi, ...
  TransitionAnimationSingle title_anim_19 { 222.f, 0.f, 40.f }; // Go ahead and try to modify ...
  
  TransitionAnimationInOut title_anim_20 { 262.f, 0.f, 2.f, 6.f, 8.f }; // Tis the season / to be folly
  TransitionAnimationInOut title_anim_21 { 272.f, 0.f, 2.f, 6.f, 8.f }; // Falalala la
  TransitionAnimationInOut title_anim_22 { 284.f, 0.f, 2.f, 6.f, 8.f }; // la la la la
  
  TransitionAnimationInOut title_anim_23 { 5.f, 0.f, 2.f, 6.f, 8.f }; // A child was born / on Christmas Day
  TransitionAnimationInOut title_anim_24 { 20.f, 0.f, 2.f, 6.f, 8.f }; // A king that in / a manger lay
  TransitionAnimationInOut title_anim_25 { 35.f, 0.f, 2.f, 6.f, 8.f }; // Jesus Christ / a name so bright
  TransitionAnimationInOut title_anim_26 { 50.f, 0.f, 2.f, 6.f, 8.f }; // Lit up the world / with such might
  
  TransitionAnimationSingle title_anim_27 { 95.f, 0.f, 2.f }; // The End!
  
  std::function<int(float)> f_r = [](float t)
  {
    return math::roundI(6.f-5.f*std::sin(4.f*(t+0.5f)));
  };
  std::function<int(float)> f_c = [](float t)
  {
    return math::roundI(-8.f+100.f*std::cos(3.05f*t)-20.f*std::sin(7.f*t));
  };
  
  bool m_use_texts = true;
  
  bool use_dynamics_system = true;
  bool dbg_draw_rigid_bodies = false;
  bool dbg_draw_sprites = false;
  bool draw_sprites = true;
  bool dbg_draw_broad_phase = false;
  bool dbg_draw_narrow_phase = false;
  
  OneShot trg_scene_1_end;
  OneShot trg_scene_2_start;
  float scene_2_start_time = 8*60.f;
  float dt_scene_transition = 0.2f;
  
  virtual void update() override
  {
    if (m_use_texts)
      update_texts();
  
    if (get_sim_time_s() < scene_2_start_time)
    {
      update_fireplace();
      
      if (use_dynamics_system)
      {
        dyn_sys.update(get_sim_time_s(), get_sim_dt_s(), get_anim_count(0));
        coll_handler.update();
      }
      
      auto key = t8::get_char_key(kpdp.transient);
      if (key == ' ')
        scene_2_start_time = get_sim_time_s();
      
      update_moon();
      
      update_shadows_and_lighting();
      
      update_meteors();
      
      update_critters();
      
      auto wind_speed = update_wind();
      smoke_vel_c = m_fire_wind_speed_factor * wind_speed * 0.65f;
      
      update_snowflakes(wind_speed);
      
      if (dbg_draw_rigid_bodies)
        dyn_sys.draw_dbg(sh);
      if (dbg_draw_sprites)
        sprh.draw_dbg_pts(sh, get_anim_count(0));
      if (dbg_draw_narrow_phase)
        coll_handler.draw_dbg_narrow_phase(sh);
      if (draw_sprites)
      {
        sprite_lake->flip_ud(0);
        sprh.draw(sh, get_anim_count(0));
        sprite_lake->flip_ud(0);
      }
      if (dbg_draw_sprites)
        sprh.draw_dbg_bb(sh, get_anim_count(0));
      if (dbg_draw_broad_phase)
        coll_handler.draw_dbg_broad_phase(sh, 0);
      
      update_lake();
    }
    else
    {
      auto scene_2_time = get_sim_time_s() - scene_2_start_time;
      if (trg_scene_1_end.once())
        sprh.clear();
      else if (math::in_range(scene_2_time, 0.f*dt_scene_transition, 1.f*dt_scene_transition, Range::ClosedOpen))
        set_screen_bg_color_default(Color16::LightGray);
      else if (math::in_range(scene_2_time, 1.f*dt_scene_transition, 2.f*dt_scene_transition, Range::ClosedOpen))
        set_screen_bg_color_default(Color16::DarkGray);
      else if (math::in_range(scene_2_time, 2.f*dt_scene_transition, 3.f*dt_scene_transition, Range::ClosedOpen))
        set_screen_bg_color_default(Color16::Black);
      else if (math::in_range(scene_2_time, 3.f*dt_scene_transition, 4.f*dt_scene_transition, Range::ClosedOpen))
        set_screen_bg_color_default(Color16::DarkGray);
      else if (math::in_range(scene_2_time, 4.f*dt_scene_transition, 5.f*dt_scene_transition, Range::ClosedOpen))
        set_screen_bg_color_default(Color16::LightGray);
      else
      {
        if (trg_scene_2_start.once())
        {
          set_screen_bg_color_default(Color16::Black);
          
          sprite_ground = sprh.create_bitmap_sprite("ground");
          sprite_ground->layer_id = 3;
          sprite_ground->pos = { sh.num_rows() - ground_height, 0 };
          sprite_ground->init(ground_height, sh.num_cols());
          sprite_ground->create_frame(0);
          sprite_ground->fill_sprite_chars(0, ':');
          sprite_ground->fill_sprite_fg_colors(0, Color16::DarkGray);
          sprite_ground->fill_sprite_bg_colors(0, Color16::DarkYellow);
          
          // Black,         //  0
          // DarkRed,       //  1
          // DarkGreen,     //  2
          // DarkYellow,    //  3
          // DarkBlue,      //  4
          // DarkMagenta,   //  5
          // DarkCyan,      //  6
          // LightGray,     //  7
          // DarkGray,      //  8
          // Red,           //  9
          // Green,         // 10
          // Yellow,        // 11
          // Blue,          // 12
          // Magenta,       // 13
          // Cyan,          // 14
          // White          // 15
          
          sprite_manger = sprh.create_bitmap_sprite("manger");
          sprite_manger->layer_id = 4;
          sprite_manger->pos = { sh.num_rows() - ground_height - 3, sh.num_cols()/2 - 5 };
          sprite_manger->init(5, 7);
          sprite_manger->create_frame(0);
          sprite_manger->set_sprite_chars_from_strings(0,
            R"(  +---+)",
            R"( /o<</|)",
            R"(+---+ |)",
            R"(| v |/ )",
            R"(+---+  )"
          );
          sprite_manger->set_sprite_fg_colors(0,
            -1, -1, 11, 11, 11, 11, 11,
            -1, 11, 13, 13, 13, 11, 11,
            11, 11, 11, 11, 11, 11, 11,
            11, 11,  1, 11, 11, 11, -1,
            11, 11, 11, 11, 11, -1, -1
          );
          sprite_manger->set_sprite_bg_colors(0,
            -3, -3,  3,  3,  3,  3,  3,
            -3,  3, 15, 15, 15,  3,  3,
             3,  3,  3,  3,  3,  3,  3,
             3,  3,  3,  3,  3,  3, -3,
             3,  3,  3,  3,  3, -3, -3
          );
          
          sprite_camel = sprh.create_bitmap_sprite("camel");
          sprite_camel->layer_id = 3;
          sprite_camel->pos = { 20, -20 };
          sprite_camel->init(6, 19);
          sprite_camel->create_frame(0);
          sprite_camel->set_sprite_chars_from_strings(0,
            R"(     ,,  ,,    ^__ )",
            R"(  ._/  \/  \__/  _})",
            R"( J(             /  )",
            R"(   \ \ \^^\ \ \'   )",
            R"(   / / /  / / /    )",
            R"(   ##D#D  ##D#D    )"
          );
          sprite_camel->fill_sprite_fg_colors(0, Color16::Yellow);
          sprite_camel->set_sprite_bg_colors(0,
            -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,  3, -3, -3, -3,
            -3, -3, -3, -3, -3,  3,  3,  3, -3,  3,  3,  3, -3, -3,  3,  3,  3,  3,  3,
            -3, -3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, -3, -3,
            -3, -3, -3, -3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, -3, -3, -3, -3,
            -3, -3, -3, -3,  3,  3,  3,  3, -3, -3, -3,  3,  3,  3,  3, -3, -3, -3, -3,
            -3, -3, -3,  3,  3,  3,  3,  3, -3, -3,  3,  3,  3,  3,  3, -3, -3, -3, -3
          );
          sprite_camel->create_frame(1);
          sprite_camel->set_sprite_chars_from_strings(1,
            R"(     ,,  ,,    ^__ )",
            R"(  ._/  \/  \__/  _})",
            R"( J(             /  )",
            R"(   \ \ \^^\ \ \'   )",
            R"(    \' /   \' /    )",
            R"(   ##D#D  ##D#D    )"
          );
          sprite_camel->fill_sprite_fg_colors(1, Color16::Yellow);
          sprite_camel->set_sprite_bg_colors(1,
            -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,  3, -3, -3, -3,
            -3, -3, -3, -3, -3,  3,  3,  3, -3,  3,  3,  3, -3, -3,  3,  3,  3,  3,  3,
            -3, -3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, -3, -3,
            -3, -3, -3, -3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, -3, -3, -3, -3,
            -3, -3, -3, -3,  3,  3,  3,  3, -3, -3, -3,  3,  3,  3,  3, -3, -3, -3, -3,
            -3, -3, -3,  3,  3,  3,  3,  3, -3, -3,  3,  3,  3,  3,  3, -3, -3, -3, -3
          );
          sprite_camel->func_calc_anim_frame = [this, &scene_2_time](auto sim_frame)
          {
            if (anim_camel.in_range(scene_2_time))
              return (sim_frame / 5) % 2;
            return 0;
          };
          
          sprite_house = sprh.create_bitmap_sprite("house");
          sprite_house->layer_id = 3;
          sprite_house->pos = { sh.num_rows() - ground_height - 10, sh.num_cols()/2 - 16 };
          sprite_house->init(10, 27);
          sprite_house->create_frame(0);
          sprite_house->set_sprite_chars_from_strings(0,
            R"(             ^             )",
            R"(           _/ \_           )",
            R"(         _/     \_         )",
            R"(       _/         \_       )",
            R"(     _/     +++     \_     )",
            R"(   _/                 \_   )",
            R"( _/                     \_ )",
            R"('|     |==+|             |')",
            R"( |     | / |             | )",
            R"( |     |+==|             | )"
          );
          sprite_house->fill_sprite_fg_colors(0, Color16::DarkRed);
          sprite_house->fill_sprite_fg_colors(0, { 7, 7, 3, 5 }, Color16::DarkGray);
          {
            int b1 = 3;
            int b2 = 9;
            int b3 = 0;
            sprite_house->set_sprite_bg_colors(0,
              -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, b2, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
              -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, b2, b1, b2, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
              -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, b2, b2, b1, b1, b1, b2, b2, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
              -3, -3, -3, -3, -3, -3, -3, -3, b2, b2, b1, b1, b1, b1, b1, b1, b1, b2, b2, -3, -3, -3, -3, -3, -3, -3, -3,
              -3, -3, -3, -3, -3, -3, b2, b2, b1, b1, b1, b1, b3, b3, b3, b1, b1, b1, b1, b2, b2, -3, -3, -3, -3, -3, -3,
              -3, -3, -3, -3, b2, b2, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b2, b2, -3, -3, -3, -3,
              -3, -3, b2, b2, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b2, b2, -3, -3,
              b2, b2, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b2, b2,
              -3, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, -3,
              -3, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, b1, -3
            );
            sprite_house->fill_sprite_fg_colors_horiz(0, 4, 12, 14, Color16::DarkYellow);
          }
          
          sprite_palm_tree = sprh.create_bitmap_sprite("palm_tree");
          sprite_palm_tree->layer_id = 3;
          sprite_palm_tree->pos = { sh.num_rows() - ground_height - 6, rnd::rand_int(-4, 2) };
          sprite_palm_tree->init(6, 12);
          sprite_palm_tree->create_frame(0);
          sprite_palm_tree->set_sprite_chars_from_strings(0,
            R"( //==\/==\\ )",
            R"( |(  \/  )| )",
            R"(     #      )",
            R"(    #       )",
            R"(    #       )",
            R"(     #      )"
          );
          auto set_palm_tree_fg_colors = [this](int frame_id)
          {
            int leaf = 10;
            int stem = 1;
            sprite_palm_tree->set_sprite_fg_colors(frame_id,
              leaf, leaf, leaf, leaf, leaf, leaf, leaf, leaf, leaf, leaf, leaf, leaf,
              leaf, leaf, leaf, leaf, leaf, leaf, leaf, leaf, leaf, leaf, leaf, leaf,
              leaf, leaf, stem, stem, stem, stem, stem, stem, stem, leaf, leaf, leaf,
              stem, stem, stem, stem, stem, stem, stem, stem, stem, stem, stem, stem,
              stem, stem, stem, stem, stem, stem, stem, stem, stem, stem, stem, stem,
              stem, stem, stem, stem, stem, stem, stem, stem, stem, stem, stem, stem
            );
          };
          set_palm_tree_fg_colors(0);
          {
            int leaf = 2;
            int stem = 11;
            sprite_palm_tree->set_sprite_bg_colors(0,
                -3,   -3, leaf, leaf, leaf,   -3,   -3, leaf, leaf, leaf,   -3,   -3,
                -3,   -3,   -3,   -3,   -3, leaf, leaf,   -3,   -3,   -3,   -3,   -3,
                -3,   -3,   -3,   -3,   -3, stem,   -3,   -3,   -3,   -3,   -3,   -3,
                -3,   -3,   -3,   -3, stem,   -3,   -3,   -3,   -3,   -3,   -3,   -3,
                -3,   -3,   -3,   -3, stem,   -3,   -3,   -3,   -3,   -3,   -3,   -3,
                -3,   -3,   -3,   -3,   -3, stem,   -3,   -3,   -3,   -3,   -3,   -3
            );
          }
          sprite_palm_tree->create_frame(1);
          sprite_palm_tree->set_sprite_chars_from_strings(1,
            R"(   //=\\/\\.)",
            R"(  '"  \/ ))|)",
            R"(      #  ")|)",
            R"(     #      )",
            R"(    #       )",
            R"(     #      )"
          );
          set_palm_tree_fg_colors(1);
          {
            int leaf = 2;
            int stem = 11;
            sprite_palm_tree->set_sprite_bg_colors(1,
                -2,   -2,   -2,   -2, leaf, leaf, leaf,   -2,   -2, leaf, leaf,   -2,
                -2,   -2,   -2,   -2,   -2, leaf, leaf,   -2,   -2, leaf, leaf,   -2,
                -2,   -2,   -2,   -2,   -2,   -2, stem,   -2,   -2,   -2, leaf,   -2,
                -2,   -2,   -2,   -2,   -2, stem,   -2,   -2,   -2,   -2,   -2,   -2,
                -2,   -2,   -2,   -2, stem,   -2,   -2,   -2,   -2,   -2,   -2,   -2,
                -2,   -2,   -2,   -2,   -2, stem,   -2,   -2,   -2,   -2,   -2,   -2
            );
          }
          sprite_palm_tree->create_frame(2);
          sprite_palm_tree->set_sprite_chars_from_strings(2,
            R"(//==\/==\\  )",
            R"(|(  \/  )|  )",
            R"(:   #       )",
            R"(    #       )",
            R"(    #       )",
            R"(     #      )"
          );
          set_palm_tree_fg_colors(2);
          {
            int leaf = 2;
            int stem = 11;
            sprite_palm_tree->set_sprite_bg_colors(2,
                -3, leaf, leaf, leaf,   -3,   -3, leaf, leaf, leaf,   -3,   -3,   -3,
                -3,   -3,   -3,   -3, leaf, leaf,   -3,   -3,   -3,   -3,   -3,   -3,
                -3,   -3,   -3,   -3, stem,   -3,   -3,   -3,   -3,   -3,   -3,   -3,
                -3,   -3,   -3,   -3, stem,   -3,   -3,   -3,   -3,   -3,   -3,   -3,
                -3,   -3,   -3,   -3, stem,   -3,   -3,   -3,   -3,   -3,   -3,   -3,
                -3,   -3,   -3,   -3,   -3, stem,   -3,   -3,   -3,   -3,   -3,   -3
            );
          }
          auto f_palm_tree_anim_sub = [](int anim_frame)
          {
            // #NOTE: I don't need to do it like this, but I thought it would be a fun challenge.
            // 0, 1, 2, 3, 0 -> 0, 1, 0, 2, 0
            // 0, 1, 2, 3, 0 % 2 -> 0, 1, 0, 1, 0 : A
            // 0, 1, 2, 3, 0 / 2 -> 0, 0, 1, 1, 0 : B
            //                      0, 1, 0, 2, 0
            // !A -> A + B
            int A = anim_frame % 2;
            int B = anim_frame / 2;
            return !A ? (A + B) % 3 : 0;
          };
          sprite_palm_tree->func_calc_anim_frame = [f_palm_tree_anim_sub](auto sim_frame)
          {
            int anim_frame = sim_frame / 8;
            return f_palm_tree_anim_sub(anim_frame);
          };
          sprite_palm_tree_arr = sprh.clone_sprite_array<3>("palm_tree", "palm_tree");
          for (int i = 0; i < stlutils::sizeI(sprite_palm_tree_arr); ++i)
          {
            auto* sprite = sprite_palm_tree_arr[i];
            auto* offs = &palm_tree_anim_offs[i];
            sprite->func_calc_anim_frame = [f_palm_tree_anim_sub, offs](auto sim_frame)
            {
              int anim_frame = (sim_frame + *offs) / 8 ;
              return f_palm_tree_anim_sub(anim_frame);
            };
            sprite->pos.c = 17 + 24*i + rnd::rand_int(-2, +2);
          }
          
          sprite_maria = sprh.create_bitmap_sprite("maria");
          sprite_maria->layer_id = 5;
          sprite_maria->pos = { sh.num_rows() - ground_height - 4, sh.num_cols()/2 + 2 };
          sprite_maria->init(7, 4);
          sprite_maria->create_frame(0);
          sprite_maria->set_sprite_chars_from_strings(0,
            R"( _  )",
            R"(( \ )",
            R"( ||})",
            R"( || )",
            R"(/  \)",
            R"(----)",
            R"( VV )"
          );
          sprite_maria->set_sprite_fg_colors(0,
            -1,  0, -1, -1,
            13, 13,  0, -1,
            -1,  7,  7,  0,
            -1,  7,  7, -1,
            15,  7,  7,  15,
             7,  7,  7,  7,
            -1,  1,  1, -1
          );
          sprite_maria->set_sprite_bg_colors(0,
            -3, -3, -3, -3,
            -3,  5, -3, -3,
            -3, 15, 15, -3,
            -3, 15, 15, -3,
            -3, 15, 15, -3,
            15, 15, 15, 15,
            -3, -3, -3, -3
          );
          
          sprite_josef = sprh.create_bitmap_sprite("josef");
          sprite_josef->layer_id = 5;
          sprite_josef->pos = { sh.num_rows() - ground_height - 4, -20 };
          sprite_josef->init(7, 8);
          sprite_josef->create_frame(0);
          sprite_josef->set_sprite_chars_from_strings(0,
            R"( ,m   _ )",
            R"(: ') | ))",
            R"( ||\_|  )",
            R"( ||  |  )",
            R"( ||  |  )",
            R"( ||  |  )",
            R"( DD  |  )"
          );
          sprite_josef->set_sprite_fg_colors(0,
            -1,  0,  0, -1, -1,  1,  1,  1,
             0, 13, 14, 13, -1,  1, -1,  1,
            -1,  7,  7,  7,  7,  1, -1, -1,
            -1,  7,  7, -1, -1,  1, -1, -1,
            -1,  7,  7, -1, -1,  1, -1, -1,
            -1,  7,  7, -1, -1,  1, -1, -1,
            -1,  0,  0, -1, -1,  1, -1, -1
          );
          sprite_josef->set_sprite_bg_colors(0,
            -3, -3, -3, -3, -3, -3, -3, -3,
            -3,  5,  5, -3, -3, -3, -3, -3,
            -3, 15, 15, -3, -3, -3, -3, -3,
            -3, 15, 15, -3, -3, -3, -3, -3,
            -3, 15, 15, -3, -3, -3, -3, -3,
            -3, 15, 15, -3, -3, -3, -3, -3,
            -3, -3, -3, -3, -3, -3, -3, -3
          );
          {
            t8::Rectangle bb { 6, 1, 1, 2 };
            sprite_josef->clone_frame(1, 0);
            sprite_josef->set_sprite_chars(1, bb, '\'', 'D');
            sprite_josef->clone_frame(2, 0);
            sprite_josef->set_sprite_chars(2, bb, 'D', '\'');
          }
          sprite_josef->func_calc_anim_frame = [this, &scene_2_time](auto sim_frame)
          {
            if (anim_josef.in_range(scene_2_time))
              return 1 + ((sim_frame / 5) % 2);
            return 0;
          };
          
          sprite_shepherd = sprh.create_bitmap_sprite("shepherd");
          sprite_shepherd->layer_id = 5;
          sprite_shepherd->pos = { sh.num_rows() - ground_height - 4, 90 };
          sprite_shepherd->init(6, 7);
          sprite_shepherd->create_frame(0);
          sprite_shepherd->set_sprite_chars_from_strings(0,
            R"( _ {J  )",
            R"(| )||  )",
            R"(|_/||\ )",
            R"(|  || ')",
            R"(|  ||  )",
            R"(|  CC  )"
          );
          sprite_shepherd->set_sprite_fg_colors(0,
            -1,  1, -1, 13,  8, -1, -1,
             1, -1,  1,  7,  7, -1, -1,
             1, 13,  7,  7,  7,  7, -1,
             1, -1, -1,  7,  7, -1, 13,
             1, -1, -1,  7,  7, -1, -1,
             1, -1, -1,  0,  0, -1, -1
          );
          sprite_shepherd->set_sprite_bg_colors(0,
            -3, -3, -3,  5, -3, -3, -3,
            -3, -3, -3, 15, 15, -3, -3,
            -3, -3, -3, 15, 15, -3, -3,
            -3, -3, -3, 15, 15, -3, -3,
            -3, -3, -3, 15, 15, -3, -3,
            -3, -3, -3, -3, -3, -3, -3
          );
          {
            t8::Rectangle bb { 5, 3, 1, 2 };
            sprite_shepherd->clone_frame(1, 0);
            sprite_shepherd->set_sprite_chars(1, bb, '\'', 'C');
            sprite_shepherd->clone_frame(2, 0);
            sprite_shepherd->set_sprite_chars(2, bb, 'C', '\'');
          }
          
          sprite_shepherd_arr = sprh.clone_sprite_array<3>("shepherd", "shepherd");
          sprite_shepherd->enabled = false;
          for (int i = 0; i < stlutils::sizeI(sprite_shepherd_arr); ++i)
          {
            auto* anim = &anim_shepherd_arr[i];
            sprite_shepherd_arr[i]->func_calc_anim_frame = [&scene_2_time, anim](auto sim_frame)
            {
              if (anim->in_range(scene_2_time))
                return 1 + ((sim_frame / 5) % 2);
              return 0;
            };
          }
          
          sprite_lamb = sprh.create_bitmap_sprite("lamb");
          sprite_lamb->layer_id = 6;
          sprite_lamb->pos = { sh.num_rows() - ground_height, 90 };
          sprite_lamb->init(3, 7);
          sprite_lamb->create_frame(0);
          sprite_lamb->set_sprite_chars_from_strings(0,
            R"(   @@  )",
            R"(o:@@@@*)",
            R"(  uuuu )"
          );
          // uuuu
          // u uu
          // uu u
          sprite_lamb->set_sprite_fg_colors(0,
           -1, -1, -1,  7,  7, -1, -1,
            0,  8,  7,  7,  7,  7, 15,
           -1, -1, 13, 13, 13, 13, -1
          );
          sprite_lamb->set_sprite_bg_colors(0,
            -3, -3, -3, 15, 15, -3, -3,
            -3, -3, 15, 15, 15, 15, -3,
            -3, -3, -3, -3, -3, -3, -3
          );
          sprite_lamb->clone_frame(1, 0);
          sprite_lamb->set_sprite_chars_horiz(1, 2, 2, 5,
            'u', ' ', 'u', 'u'
          );
          sprite_lamb->clone_frame(2, 0);
          sprite_lamb->set_sprite_chars_horiz(2, 2, 2, 5,
            'u', 'u', ' ', 'u'
          );
          sprite_lamb_arr = sprh.clone_sprite_array<3>("lamb", "lamb");
          sprite_lamb->enabled = false;
          for (int i = 0; i < stlutils::sizeI(sprite_lamb_arr); ++i)
          {
            auto* anim = &anim_lamb_arr[i];
            sprite_lamb_arr[i]->func_calc_anim_frame = [&scene_2_time, anim](auto sim_frame)
            {
              if (anim->in_range(scene_2_time))
                return 1 + ((sim_frame / 5) % 2);
              return 0;
            };
            sprite_lamb_arr[i]->layer_id = 6 + i;
          }
          
          sprite_bethlehem_star = sprh.create_bitmap_sprite("bethlehem star");
          sprite_bethlehem_star->layer_id = 1;
          sprite_bethlehem_star->pos = { 3, sh.num_cols() - 13 };
          sprite_bethlehem_star->init(10, 7);
          sprite_bethlehem_star->create_frame(0);
          sprite_bethlehem_star->set_sprite_chars_from_strings(0,
            R"(   .   )",
            R"(   |   )",
            R"( . | . )",
            R"(  \v/  )",
            R"(-->*<--)",
            R"(  /^\  )",
            R"( ' | ' )",
            R"(   |   )",
            R"(   |   )",
            R"(   :   )"
          );
          sprite_bethlehem_star->set_sprite_fg_colors(0,
            -1, -1, -1, 11, -1, -1, -1,
            -1, -1, -1, 11, -1, -1, -1,
            -1, 11, -1, 11, -1, 11, -1,
            -1, -1, 11, 15, 11, -1, -1,
            11, 11, 15, 15, 15, 11, 11,
            -1, -1, 11, 15, 11, -1, -1,
            -1, 11, -1, 11, -1, 11, -1,
            -1, -1, -1, 11, -1, -1, -1,
            -1, -1, -1, 11, -1, -1, -1,
            -1, -1, -1, 11, -1, -1, -1
          );
          sprite_bethlehem_star->fill_sprite_bg_colors(0, Color16::Transparent2);
          sprite_bethlehem_star->clone_frame(1, 0);
          sprite_bethlehem_star->set_sprite_chars(1, { 2, 3, 5, 1 },
            'v', '|', '*', '|', '^'
          );
          sprite_bethlehem_star->func_calc_anim_frame = [](auto sim_frame)
          {
            return ((sim_frame / 7) % 2);
          };
          
          
          generate_star_sprites();
          
          chip_tune->stop_tune_async();
          Delay::sleep(100'000);
          chip_tune->remove_listener(this);
          chip_tune.release();
        }
        else if (scene_2_time > 6.f*dt_scene_transition && trg_scene_2_tune.once())
        {
          chip_tune = std::make_unique<beat::ChipTuneEngine>(audio, wave_gen);
          chip_tune->set_volume_slider(m_music_volume, min_dB, nl_taper);
          chip_tune->add_listener(this);
          Delay::sleep(150'000);
          play_tune("nigh_bethlehem.ct");
        }
      }
      
      if (sprite_josef != nullptr && anim_josef.in_range(scene_2_time))
      {
        auto c = anim_josef.animate(scene_2_time, -20, 29, t8x::easings::ease_out_sine);
        sprite_josef->pos.c = math::roundI(c);
      }
      else if (sprite_camel != nullptr && anim_camel.in_range(scene_2_time))
      {
        auto c = anim_camel.animate(scene_2_time, -20, 10, t8x::easings::ease_out_sine);
        sprite_camel->pos.c = math::roundI(c);
      }
      else
      {
        if (sprite_shepherd != nullptr)
        {
          for (int i = 0; i < stlutils::sizeI(sprite_shepherd_arr); ++i)
          {
            if (anim_shepherd_arr[i].in_range(scene_2_time))
            {
              auto c = anim_shepherd_arr[i].animate(scene_2_time, 90, static_cast<float>(55 + 8*i), t8x::easings::ease_out_sine);
              sprite_shepherd_arr[i]->pos.c = math::roundI(c);
            }
          }
        }
        if (sprite_lamb != nullptr)
        {
          for (int i = 0; i < stlutils::sizeI(sprite_lamb_arr); ++i)
          {
            if (anim_lamb_arr[i].in_range(scene_2_time))
            {
              auto c = anim_lamb_arr[i].animate(scene_2_time, 90, static_cast<float>(57 + 5*i), t8x::easings::ease_out_sine);
              sprite_lamb_arr[i]->pos.c = math::roundI(c);
            }
          }
        }
      }
      
      if (draw_sprites)
        sprh.draw(sh, get_anim_count(0));
      if (dbg_draw_sprites)
        sprh.draw_dbg_pts(sh, get_anim_count(0));
    }
  }
  
  virtual void draw_instructions() override
  {
    sh.write_buffer("INSTRUCTIONS", 1, 20, Color16::White);
    sh.write_buffer("============", 2, 20, Color16::White);
    sh.write_buffer("Keys:", 4, 2, Color16::White);
    sh.write_buffer("<space> : Next scene.", 5, 3, Color16::White);
    sh.write_buffer("    'p' : Pause", 6, 3, Color16::White);
    sh.write_buffer("    'q' : Quit", 7, 3, Color16::White);
    sh.write_buffer("Press space-bar to continue", 29, 25, Color16::White);
  }
  
  virtual void on_exit_instructions() override
  {
    play_tune("deck_the_halls.ct");
  }
  
  virtual void on_enter_game_loop() override
  {
    benchmark::tic();
  }
  
  virtual void on_exit_game_loop() override
  {
    auto dur_s = 1e-3f * benchmark::toc();
    
    auto fps = get_frame_count_measure() / dur_s;
    std::cout << "goal FPS = " << get_real_fps() << ", measured FPS = " << fps << std::endl;
  }
  
  virtual void on_halt_game_loop() override
  {
    chip_tune->pause();
  }
  
  virtual void on_resume_game_loop() override
  {
    chip_tune->resume();
  }
  
  virtual void on_quit() override
  {
    chip_tune->stop_tune_async();
  }
};

int main(int argc, char** argv)
{
  t8x::GameEngineParams params;
  params.screen_bg_color_default = Color16::Black;
  params.enable_title_screen = false;
  params.enable_instructions_screen = true;
  params.screen_bg_color_instructions = Color16::Black;
  
  bool use_audio = true;
  float music_volume = c_default_vol;
  bool use_texts = true;
  bool show_help = false;
  float moon_speed_factor = 1.f;
  float wind_speed_factor = 1.f;
  float snow_wind_gradient_max = 20.f;
  float snow_wind_gradient_min = 0.f;
  float fire_wind_speed_factor = 1.f;

  for (int i = 1; i < argc; ++i)
  {
    if (std::strcmp(argv[i], "--suppress_tty_output") == 0)
      params.suppress_tty_output = true;
    else if (std::strcmp(argv[i], "--suppress_tty_input") == 0)
      params.suppress_tty_input = true;
    else if (std::strcmp(argv[i], "--disable_audio") == 0)
      use_audio = false;
    else if (std::strcmp(argv[i], "--music_volume") == 0)
      music_volume = static_cast<float>(std::atof(argv[i + 1]));
    else if (std::strcmp(argv[i], "--disable_texts") == 0)
      use_texts = false;
    else if (std::strcmp(argv[i], "--moon_speed_factor") == 0)
      moon_speed_factor = static_cast<float>(std::atof(argv[i + 1]));
    else if (std::strcmp(argv[i], "--wind_speed_factor") == 0)
      wind_speed_factor = static_cast<float>(std::atof(argv[i + 1]));
    else if (std::strcmp(argv[i], "--fire_wind_speed_factor") == 0)
      fire_wind_speed_factor = static_cast<float>(std::atof(argv[i + 1]));
    else if (std::strcmp(argv[i], "--help") == 0)
      show_help = true;
  }

  if (show_help)
    use_audio = false;

  
  Game game(argc, argv, params,
            use_audio, std::clamp(music_volume, 0.f, 1.f), use_texts,
            std::max(0.f, moon_speed_factor), std::max(0.f, wind_speed_factor),
            std::clamp(snow_wind_gradient_max, 0.f, 20.f), std::clamp(snow_wind_gradient_min, 0.f, 20.f),
            std::clamp(fire_wind_speed_factor, 0.f, 1.f));
  
  for (int i = 1; i < argc; ++i)
  {
    if (i + 1 < argc && std::strcmp(argv[i], "--set_fps") == 0)
      game.set_real_fps(static_cast<float>(std::atof(argv[i + 1])));
    else if (i + 1 < argc && std::strcmp(argv[i], "--set_sim_delay_us") == 0)
      game.set_sim_delay_us(static_cast<float>(std::atof(argv[i + 1])));
  }

  if (show_help)
  {
    std::cout << "christmas_demo --help | " << std::endl;
    std::cout << "   [--suppress_tty_output]" << std::endl;
    std::cout << "   [--suppress_tty_input]" << std::endl;
    std::cout << "   [--set_fps <fps>]" << std::endl;
    std::cout << "   [--set_sim_delay_us <delay_us>]" << std::endl;
    std::cout << "   [--disable_audio]" << std::endl;
    std::cout << "   [--music_volume <music_vol>]" << std::endl;
    std::cout << "   [--disable_texts]" << std::endl;
    std::cout << "   [--moon_speed_factor <msf>]" << std::endl;
    std::cout << "   [--wind_speed_factor <wsf>]" << std::endl;
    std::cout << "   [--snow_wind_gradient_max <swgmax>]" << std::endl;
    std::cout << "   [--snow_wind_gradient_min <swgmin>]" << std::endl;
    std::cout << "   [--fire_wind_speed_factor <fwsf>]" << std::endl;
    std::cout << std::endl;
    std::cout << "  default values:" << std::endl;
    std::cout << "    <fps>       : " << game.get_real_fps() << std::endl;
    std::cout << "    <delay_us>  : " << game.get_sim_delay_us() << std::endl;
    std::cout << "    <music_vol> : " << c_default_vol << " (valid range: [0, 1])" << std::endl;
    std::cout << "    <msf>       : " << moon_speed_factor << " (valid range: [0, FLT_MAX))" << std::endl;
    std::cout << "    <wsf>       : " << wind_speed_factor << " (valid range: [0, FLT_MAX))" << std::endl;
    std::cout << "    <sgwmax>    : " << snow_wind_gradient_max << " (valid range: [0, 20))" << std::endl;
    std::cout << "    <sgwmin>    : " << snow_wind_gradient_min << " (valid range: [0, 20))" << std::endl;
    std::cout << "    <fwsf>      : " << fire_wind_speed_factor << " (valid range: [0, 1])" << std::endl;
    return EXIT_SUCCESS;
  }
  
  game.init();
  game.generate_data();
  game.run();

  return EXIT_SUCCESS;
}

