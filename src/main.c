#include <pebble.h>
#include <lightbox.h>
#include <settings.h>
	
//uncomment to disable app logging
//#undef APP_LOG
//#define APP_LOG(...)
	
#define MAX_LIGHTS 4
#define MAX_STARTUP_SEQUENCES 6
#define CURVE AnimationCurveEaseInOut
	
int animation_choices;

typedef enum { 
	Simultaneous = 0,
	Sequential = 1, 
	Staggered = 2 
}Mode;
	
static Mode mode;
int delay;
int duration;

static Layer *light_one;
static Layer *light_two;
static Layer *light_three;
static Layer *light_four;

static Window *window;
Layer *root_layer;
static BitmapLayer *mask_layer;
static GBitmap *mask;
static InverterLayer *inverter;

static PropertyAnimation *light_animation_one;
static PropertyAnimation *light_animation_two;
static PropertyAnimation *light_animation_three;
static PropertyAnimation *light_animation_four;

static GRect location_one;
static GRect location_two;
static GRect location_three;
static GRect location_four;
static GPoint centre;
static GRect rect;

static void update_timers(int step)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "updating timers (step = %d)", step);
	switch(mode)
	{
		case Simultaneous:
			delay = 0;
			duration = animation_duration;
			break;
		case Sequential:
			delay = step * (animation_duration/MAX_LIGHTS);
			duration = (animation_duration/MAX_LIGHTS);
			break;
		case Staggered:
			delay = step * (animation_duration/(2*MAX_LIGHTS));
			duration = (MAX_LIGHTS+1) * (animation_duration/(2*MAX_LIGHTS));
			break;
		default:
			delay = 0;
			duration = 0;
			break;
	}
}


static void run_animations()
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Running animations");
	delay = 0;
	duration = 0;

	update_timers(0);
	//destroy any existing animation and create a new one
	property_animation_destroy(light_animation_one);
	light_animation_one = property_animation_create_layer_frame(light_one, NULL, &location_one);
	animation_set_curve((Animation*)light_animation_one, CURVE);
	animation_set_delay((Animation*)light_animation_one, delay);
	animation_set_duration((Animation*)light_animation_one, duration);
	animation_schedule((Animation*)light_animation_one);

	update_timers(1);
	//destroy any existing animation and create a new one
	property_animation_destroy(light_animation_two);
	light_animation_two = property_animation_create_layer_frame(light_two, NULL, &location_two);
	animation_set_curve((Animation*)light_animation_two, CURVE);
	animation_set_delay((Animation*)light_animation_two, delay);
	animation_set_duration((Animation*)light_animation_two, duration);
	animation_schedule((Animation*)light_animation_two);

	update_timers(2);
	//destroy any existing animation and create a new one
	property_animation_destroy(light_animation_three);
	light_animation_three = property_animation_create_layer_frame(light_three, NULL, &location_three);
	animation_set_curve((Animation*)light_animation_three, CURVE);
	animation_set_delay((Animation*)light_animation_three, delay);
	animation_set_duration((Animation*)light_animation_three, duration);
	animation_schedule((Animation*)light_animation_three);

	update_timers(3);
	//destroy any existing animation and create a new one
	property_animation_destroy(light_animation_four);
	light_animation_four = property_animation_create_layer_frame(light_four, NULL, &location_four);
	animation_set_curve((Animation*)light_animation_four, CURVE);
	animation_set_delay((Animation*)light_animation_four, delay);
	animation_set_duration((Animation*)light_animation_four, duration);
	animation_schedule((Animation*)light_animation_four);
}

static void set_next_locations(struct tm *tick_time)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting locations");
	int hour = tick_time->tm_hour;
	int minute = tick_time->tm_min;
	APP_LOG(APP_LOG_LEVEL_DEBUG, "hour = %d", hour);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "minute = %d", minute);
	
	//put lights to a point in the centre of the screen, unused ones will stay there.
	location_one = IDLE;	
	location_two = IDLE;	
	location_three = IDLE;	
	location_four = IDLE;	
	
	if(minute <= 30 && minute != 0)
	{
		location_three = PAST;
	}
	else if(minute > 30)
	{
		location_three = TO;
		hour++;
		if(hour > 23)
		{
			hour = 0;
		} 
		minute = 30 - (minute - 30);
	}
	if(hour > 12)
	{
		hour = hour - 12;
	}
	
	switch(minute)
	{
		case 0:	if(hour != 12 && hour != 0){location_two = OCLOCK;} break;
		case 1: location_one = ONE; break;
		case 2: location_one = TWO; break;
		case 3: location_one = THREE; break;
		case 4: location_one = FOUR; break;
		case 5: location_one = FIVE; break;
		case 6: location_one = SIX; break;
		case 7: location_one = SEVEN; break;
		case 8: location_one = EIGHT; break;
		case 9: location_one = NINE; break;
		case 10: location_one = TEN; break;
		case 11: location_one = ELEVEN; break;
		case 12: location_one = TWELVE; break;
		case 13: location_one = THIR; location_two = TEEN; break;
		case 14: location_one = FOUR; location_two = TEEN; break;
		case 15: location_one = QUARTER; break;
		case 16: location_one = SIX; location_two = TEEN; break;
		case 17: location_one = SEVEN; location_two = TEEN; break;
		case 18: location_one = EIGH; location_two = TEEN; break;
		case 19: location_one = NINE; location_two = TEEN; break;
		case 20: location_one = TWENTY; break;
		case 21: location_one = TWENTY; location_two = ONE; break;
		case 22: location_one = TWENTY; location_two = TWO; break;
		case 23: location_one = TWENTY; location_two = THREE; break;
		case 24: location_one = TWENTY; location_two = FOUR; break;
		case 25: location_one = TWENTY; location_two = FIVE; break;
		case 26: location_one = TWENTY; location_two = SIX; break;
		case 27: location_one = TWENTY; location_two = SEVEN; break;
		case 28: location_one = TWENTY; location_two = EIGHT; break;
		case 29: location_one = TWENTY; location_two = NINE; break;
		case 30: location_one = HALF; break;
	}
	
	switch(hour)
	{
		case 1: if(minute == 0){location_four = ONE;}else{location_four = H_ONE;} break;
		case 2: if(minute == 0){location_four = TWO;}else{location_four = H_TWO;} break;
		case 3: if(minute == 0){location_four = THREE;}else{location_four = H_THREE;} break;
		case 4: if(minute == 0){location_four = FOUR;}else{location_four = H_FOUR;}	break;
		case 5: if(minute == 0){location_four = FIVE;}else{location_four = H_FIVE;} break;
		case 6: if(minute == 0){location_four = SIX;}else{location_four = H_SIX;} break;
		case 7: if(minute == 0){location_four = SEVEN;}else{location_four = H_SEVEN;} break;
		case 8:	if(minute == 0){location_four = EIGHT;}else{location_four = H_EIGHT;} break;
		case 9:	if(minute == 0){location_four = NINE;}else{location_four = H_NINE;} break;
		case 10: if(minute == 0){location_four = TEN;}else{location_four = H_TEN;} break;
		case 11: if(minute == 0){location_four = ELEVEN;}else{location_four = H_ELEVEN;} break;
		case 12: location_four = MIDDAY; break;
		case 0: location_four = MIDNIGHT; break;
	}
}

static void light_draw(Layer *layer, GContext *ctx)
{
	graphics_context_set_fill_color(ctx, fg_color);
	graphics_context_set_stroke_color(ctx, fg_color);
	graphics_fill_rect(ctx, GRect(0,0,144,168), 0, GCornerNone);
}

static void set_animation_type(int ratio)
{	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting animation type (ratio = %d)", ratio);
	switch(rand() % ratio)
	{
		case 0:
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Four corners");
			layer_set_frame(light_one, GRect(-144,168,144,168));
			layer_set_frame(light_two, GRect(144,-168,144,168));
			layer_set_frame(light_three, GRect(-144,-168,144,168));
			layer_set_frame(light_four, GRect(144,168,144,168));
			mode = Sequential;
			break;
		case 1:
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Center point");
			layer_set_frame(light_one, GRect(72,84,0,0));
			layer_set_frame(light_two, GRect(73,84,0,0));
			layer_set_frame(light_three, GRect(73,85,0,0));
			layer_set_frame(light_four, GRect(72,85,0,0));
			mode = Staggered;
			break;
		case 2:
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Full screen");
			layer_set_frame(light_one, GRect(0,0,144,168));
			layer_set_frame(light_two, GRect(0,0,144,168));
			layer_set_frame(light_three, GRect(0,0,144,168));
			layer_set_frame(light_four, GRect(0,0,144,168));
			mode = Simultaneous;
			break;
		case 3:
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Card deal");
			layer_set_frame(light_one, GRect(65,168,14,16));
			layer_set_frame(light_two, GRect(65,168,14,16));
			layer_set_frame(light_three, GRect(65,168,14,16));
			layer_set_frame(light_four, GRect(65,168,14,16));
			mode = Sequential;
			break;
		case 4:
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Light up");
			centre = grect_center_point(&location_one);
			layer_set_frame(light_one, GRect(centre.x,centre.y,0,0));
			centre = grect_center_point(&location_two);
			layer_set_frame(light_two, GRect(centre.x,centre.y,0,0));
			centre = grect_center_point(&location_three);
			layer_set_frame(light_three, GRect(centre.x,centre.y,0,0));
			centre = grect_center_point(&location_four);
			layer_set_frame(light_four, GRect(centre.x,centre.y,0,0));
			mode = Sequential;
			break;
		case 5:
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Drop in");
			rect = location_one;
			rect.origin.y = rect.origin.y - 168;
			layer_set_frame(light_one, rect);
			rect = location_two;
			rect.origin.y = rect.origin.y - 168;
			layer_set_frame(light_two, rect);
			rect = location_three;
			rect.origin.y = rect.origin.y - 168;
			layer_set_frame(light_three, rect);
			rect = location_four;
			rect.origin.y = rect.origin.y - 168;
			layer_set_frame(light_four, rect);
			mode = Sequential;
			break;
		default:
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Normal transition animation");
			mode = Simultaneous;
			break;
	}
}
	
static void init_layers()
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "initialising layers");
	light_one = layer_create(IDLE);
	layer_set_clips(light_one, true);
	layer_set_update_proc(light_one, light_draw);
	layer_add_child(root_layer, light_one);

	light_two = layer_create(IDLE);
	layer_set_clips(light_two, true);
	layer_set_update_proc(light_two, light_draw);
	layer_add_child(root_layer, light_two);

	light_three = layer_create(IDLE);
	layer_set_clips(light_three, true);
	layer_set_update_proc(light_three, light_draw);
	layer_add_child(root_layer, light_three);

	light_four = layer_create(IDLE);
	layer_set_clips(light_four, true);
	layer_set_update_proc(light_four, light_draw);
	layer_add_child(root_layer, light_four);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{
	APP_LOG(APP_LOG_LEVEL_INFO, "Tick!");
	APP_LOG(APP_LOG_LEVEL_WARNING, "Heap used at start of tick: %u", heap_bytes_used());
	APP_LOG(APP_LOG_LEVEL_WARNING, "Heap free at start of tick: %u", heap_bytes_free());

	set_next_locations(tick_time);
	set_animation_type(animation_choices);
	run_animations();
	animation_choices = MAX_STARTUP_SEQUENCES * 5;
	
	if(vibrate && tick_time->tm_min == 0)
	{
		vibes_double_pulse();
	}
}

void handle_init(void)
{
	APP_LOG(APP_LOG_LEVEL_WARNING, "Heap used at start of initialisation: %u", heap_bytes_used());
	APP_LOG(APP_LOG_LEVEL_WARNING, "Heap free at start of initialisation: %u", heap_bytes_free());
	load_settings();
	
	window = window_create();
	window_set_background_color(window, bg_color);
	window_stack_push(window, true);
	root_layer = window_get_root_layer(window);

	time_t now = time(NULL);
	struct tm *time = localtime(&now);
	srand(now);
	
	animation_choices = MAX_STARTUP_SEQUENCES;
	APP_LOG(APP_LOG_LEVEL_DEBUG, "animation choices = %d", animation_choices);
	set_next_locations(time);
	init_layers();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "initialising mask layer");
	mask = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MASK_BLACK);
	mask_layer = bitmap_layer_create(layer_get_frame(root_layer));
	bitmap_layer_set_bitmap(mask_layer, mask);
	bitmap_layer_set_compositing_mode(mask_layer, GCompOpClear);
	layer_add_child(root_layer, bitmap_layer_get_layer(mask_layer));

	APP_LOG(APP_LOG_LEVEL_DEBUG, "initialising inverter layer");
	inverter = inverter_layer_create(GRect(0,0,144,168));
	layer_add_child(root_layer, inverter_layer_get_layer(inverter));
	layer_set_hidden(inverter_layer_get_layer(inverter), !invert_colors); //hide layer to not invert colors
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Subscribing to minute ticks");
	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);        
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Initialisation complete");

	APP_LOG(APP_LOG_LEVEL_WARNING, "Heap used at end of initialisation: %u", heap_bytes_used());
	APP_LOG(APP_LOG_LEVEL_WARNING, "Heap free at end of initialisation: %u", heap_bytes_free());
}

void handle_deinit(void) 
{
	save_settings();
	animation_unschedule_all();
	
	layer_destroy(light_one);
	layer_destroy(light_two);
	layer_destroy(light_three);
	layer_destroy(light_four);
	
	property_animation_destroy(light_animation_one);
	property_animation_destroy(light_animation_two);
	property_animation_destroy(light_animation_three);
	property_animation_destroy(light_animation_four);
	
	bitmap_layer_destroy(mask_layer);
	inverter_layer_destroy(inverter);
	window_destroy(window);
}

int main(void) 
{
	handle_init();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Entering main app event loop");
	app_event_loop();
	handle_deinit();
}