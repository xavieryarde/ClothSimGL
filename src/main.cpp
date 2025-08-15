#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "simulation.hpp"

int main(int argc, char* argv[]) {
    Simulation simulation = Simulation();

    if (!simulation.init()) {
        SDL_Log("Error Initializing\n");
        return -1;
    }
    
    simulation.run();
	return 0;
}
