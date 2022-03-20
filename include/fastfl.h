#ifndef FASTFL_H
#define FASTFL_H

typedef struct FFL_Settings FFL_Settings;
typedef struct FFL_Graph FFL_Graph;

struct FFL_Settings {
	float spring_strength;
	float repulsion_strength;
};

void ffl_compute(FFL_Graph *graph, const FFL_Settings *settings);

#endif /* FASTFL_H */

