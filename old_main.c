#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <SDL2/SDL.h>

#define	WINDOW_W 970
#define WINDOW_H 970
#define MAX_INT 9223372036854775807
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef struct		s_xy
{
	int				x;
	int				y;
}					t_xy;

typedef	struct		s_vec
{
	double			x;
	double			y;
	double			z;
}					t_vec;

typedef struct		s_col
{
	int				r;
	int				g;
	int				b;
	int				a;
}					t_col;

typedef	struct		s_sphere
{
	t_vec			center;
	double			radius;
	t_col			color;
}					t_sphere;

typedef	struct		s_env
{
	SDL_Window		*window;
	SDL_Renderer	*renderer;
	SDL_Event		e;
	t_vec			camera;
	double			viewport_size;
	double			projection_plane_z;
}					t_env;

/*
LINEAR ALGEBRA
*/

double	dot_product(t_vec v1, t_vec v2)
{
	return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

t_vec	substract(t_vec v1, t_vec v2)
{
	t_vec new;

	new.x = v1.x - v2.x;
	new.y = v1.y - v2.y;
	new.z = v1.z - v2.z;
	return (new);
}

double	length(t_vec vec)
{
	return (sqrt(dot_product(vec, vec)));
}

t_vec	multiple(double k, t_vec vec)
{
	t_vec	new;

	new.x = k * vec.x;
	new.y = k * vec.y;
	new.z = k * vec.z;
	return (new);
}

t_vec	add(t_vec v1, t_vec v2)
{
	t_vec	new;

	new.x = v1.x + v2.x;
	new.y = v1.y + v2.y;
	new.z = v1.z + v2.z;
	return (new);
}

t_col	clamp(t_vec v1)
{
	t_col	new;

	new.r = MIN(255, MAX(0, v1.x));
	new.g = MIN(255, MAX(0, v1.y));
	new.b = MIN(255, MAX(0, v1.z));
	new.a = 255;
	return (new);
}

/*	END	*/
void	exit_fail(const char *error, int num)
{
	printf("[%s][%d]\n", error, num);
	exit(0);
}

void	init(t_env *env)
{
	if ((SDL_Init(SDL_INIT_EVERYTHING) == -1))
		exit_fail(SDL_GetError(), 1);
	if ((SDL_CreateWindowAndRenderer(WINDOW_W, WINDOW_H, 0, &env->window, &env->renderer) == -1))
		exit_fail(SDL_GetError(), 2);
	//if (SDL_SetRenderDrawColor(env->renderer, 255, 0, 0, 255) < 0)
	//	exit_fail(SDL_GetError(), 3);
	env->camera.x = 0;
	env->camera.y = 0;
	env->camera.z = 0;
	env->viewport_size = 10;
	env->projection_plane_z = 1;
}

void	draw_rect(t_env env)
{

	SDL_Rect	rect;
	rect.x = 10;
	rect.y = 10;
	rect.h = 50;
	rect.w = 50;
	if ((SDL_RenderFillRect(env.renderer, &rect) < 0))
		exit_fail(SDL_GetError(), 4);
}

void	draw_line(t_env env)
{
	if (SDL_RenderDrawLine(env.renderer, 10, 10, 100, 100) < 0)
		exit_fail(SDL_GetError(), 5);
}

void	destroy(t_env env)
{
	SDL_DestroyRenderer(env.renderer);
	SDL_DestroyWindow(env.window);
	SDL_Quit();
	printf("[DONE]\n");
}

void	intersect_ray(t_vec origin, t_vec diraction, t_sphere sphere, t_vec *ts)
{
	t_vec oc = substract(origin, sphere.center);
	double k1 = dot_product(diraction, diraction);
	double k2 = 2 * dot_product(oc, diraction);
	double k3 = dot_product(oc, oc) - sphere.radius * sphere.radius;

	double diskriminant = k2 * k2 - 4.0 * k1 * k3;
	if (diskriminant < 0)
	{
		ts->x = MAX_INT;
		ts->y = MAX_INT;
		return ;
	}
	ts->x = (-k2 + sqrt(diskriminant)) / (2.0 * k1);
	ts->y = (-k2 - sqrt(diskriminant)) / (2.0 * k1);
}

void	trace_ray(t_vec camera, t_vec diraction, t_col *color)
{
	double closest = MAX_INT;
	t_sphere closest_sphere;
	int done;
	int	i;
	t_vec ts;
	t_sphere	spheres[3] = 
	{
		// X Y Z
		{{0, -1, 3}, 1, {255, 0, 0, 255}},//red
		{{0, 0, 0}, 3, {0, 0, 255, 255}}, // blue
		{{0, 0, 4}, 1, {0, 255, 0, 255}}, // green
	};

	i = -1;
	// printf("[%d]\n", SDL_MAX_SINT32);
	while (++i < 3)
	{
		intersect_ray(camera, diraction, spheres[i], &ts);
		if (ts.x < closest && 1 < ts.x && ts.x < MAX_INT)
		{
			closest = ts.x;
			closest_sphere = spheres[i];
			done = 1;
		}
		if (ts.y < closest && 1 < ts.y && ts.y < MAX_INT)
		{
			closest = ts.y;
			closest_sphere = spheres[i];
			done = 1;
		}
	}
	if (!done)
	{
		color->r = 255;
		color->g = 255;
		color->b = 255;
		color->a = 255;
	}
	else
	{
		color->r = closest_sphere.color.r;
		color->g = closest_sphere.color.g;
		color->b = closest_sphere.color.b;
	}
}

void	canvas_to_view(t_vec *diraction, t_env env, t_xy xy)
{
	diraction->x = (double)xy.x * env.viewport_size / (double)WINDOW_W;
	diraction->y = (double)xy.y * env.viewport_size / (double)WINDOW_W;
	diraction->z = (double)env.projection_plane_z;
}

void	put_pixel(int x, int y, t_col color, t_env env)
{
	// printf("[%d][%d][%d]\n", color.r, color.g, color.b);
	x = WINDOW_W / 2 + x;
	y = WINDOW_H / 2 - y + 1;
	if (x < 0 || x >= WINDOW_W || y < 0 || y >= WINDOW_H)
		return ;
	SDL_SetRenderDrawColor(env.renderer, color.r, color.g, color.b, 255);
	SDL_RenderDrawPoint(env.renderer, x, y);
}

void	draw_sphere(t_env env)
{
	t_xy	xy;
	t_vec	diraction;
	t_col	color;

	printf("[%ld]\n", sizeof(double));
	xy.x = -(WINDOW_W / 2) - 1;
	while (++xy.x < WINDOW_W / 2)
	{
		xy.y = -(WINDOW_H / 2) - 1;
		while (++xy.y < WINDOW_H / 2)
		{
			canvas_to_view(&diraction, env, xy);
			trace_ray(env.camera, diraction, &color);
			// printf("[%d][%d][%d]\n", color.r, color.g, color.b);
			put_pixel(xy.x, xy.y, color, env);
		}
	}
}

int		main(int ac, char **av)
{
	t_env	env;
	int		q;

	q = 0;
	init(&env);
	// draw_line(env);
	// draw_rect(env);
	draw_sphere(env);
	SDL_RenderPresent(env.renderer);
	while (!q)
		while(SDL_PollEvent(&env.e))
			env.e.type == SDL_QUIT ? q = 1 : 0;
	destroy(env);
	return (0);
}