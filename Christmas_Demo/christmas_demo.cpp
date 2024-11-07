//
//  christmas_demo.cpp
//  Christmas_Demo
//
//  Created by Rasmus Anthin on 2024-11-07.
//

#include <Termin8or/GameEngine.h>
#include <iostream>

class Game : public GameEngine<>
{
public:
  Game(int argc, char** argv, const GameEngineParams& params)
    : GameEngine(argv[0], params)
  {
  }
  
  virtual void generate_data() override
  {
  
  }
  
private:
  
  virtual void update() override
  {
  
  }
};

int main(int argc, char** argv)
{
  GameEngineParams params;
  params.screen_bg_color_default = Color::Blue;
  params.screen_bg_color_title = Color::Blue;
  params.screen_bg_color_instructions = Color::Black;
  
  Game game(argc, argv, params);
  
  game.init();
  game.generate_data();
  game.run();

  return EXIT_SUCCESS;
}
