#include <pebble.h>

#define MSG_SHOW_NO_PHONE 0
#define MSG_SHOW_BATTERY 1
#define MSG_VIBE 2
#define MSG_SIMPLE 3
#define MSG_SECONDS 4

static Window *window;

static GBitmap *hour_bmp_black;
static GBitmap *min_bmp_black;
static GBitmap *sec_bmp_black;
static GBitmap *hour_bmp_white;
static GBitmap *min_bmp_white;
static GBitmap *sec_bmp_white;
static GBitmap *bg_bmp;
static GBitmap *bg_bmp_simple;
static GBitmap *battery_bmp_black;
static GBitmap *battery_bmp_white;
static GBitmap *battery_charging_bmp_black;
static GBitmap *battery_charging_bmp_white;
static GBitmap *no_phone_bmp_black;
static GBitmap *no_phone_bmp_white;

static BitmapLayer* bg_layer;
static RotBitmapLayer* hour_hand_layer_black;
static RotBitmapLayer* minute_hand_layer_black;
static RotBitmapLayer* second_hand_layer_black;
static RotBitmapLayer* hour_hand_layer_white;
static RotBitmapLayer* minute_hand_layer_white;
static RotBitmapLayer* second_hand_layer_white;
static Layer *battery_layer;
static Layer *no_phone_layer;

bool config_show_no_phone = true;
bool config_show_battery = true;
bool config_vibe = false;
bool config_simple = false;
bool config_seconds = false;

static void update_time()
{
	time_t t = time(NULL);
	struct tm *tick_time = localtime(&t);
	rot_bitmap_layer_set_angle(hour_hand_layer_black, TRIG_MAX_ANGLE * (tick_time->tm_hour%12) / 12 + TRIG_MAX_ANGLE * tick_time->tm_min / 720);
	rot_bitmap_layer_set_angle(hour_hand_layer_white, TRIG_MAX_ANGLE * (tick_time->tm_hour%12) / 12 + TRIG_MAX_ANGLE * tick_time->tm_min / 720);
	rot_bitmap_layer_set_angle(minute_hand_layer_black, TRIG_MAX_ANGLE * tick_time->tm_min / 60);
	rot_bitmap_layer_set_angle(minute_hand_layer_white, TRIG_MAX_ANGLE * tick_time->tm_min / 60);
	if (config_seconds)
	{
		rot_bitmap_layer_set_angle(second_hand_layer_black, TRIG_MAX_ANGLE * tick_time->tm_sec / 60);
		rot_bitmap_layer_set_angle(second_hand_layer_white, TRIG_MAX_ANGLE * tick_time->tm_sec / 60);
	}
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
}

void battery_update_callback(Layer *layer, GContext *ctx)
{
		if (config_show_battery)
		{
				GRect image_rect = battery_bmp_white->bounds;
				BatteryChargeState charge_state = battery_state_service_peek();
				if (!charge_state.is_charging)
				{
						graphics_context_set_compositing_mode(ctx, GCompOpOr);
						graphics_draw_bitmap_in_rect(ctx, battery_bmp_white, image_rect);
						graphics_context_set_compositing_mode(ctx, GCompOpClear);
						graphics_draw_bitmap_in_rect(ctx, battery_bmp_black, image_rect);
						graphics_context_set_fill_color(ctx, GColorBlack);
						graphics_fill_rect(ctx, GRect(4, 4, charge_state.charge_percent / 10, 6), 0, GCornerNone);
				} else {
						graphics_context_set_compositing_mode(ctx, GCompOpOr);
						graphics_draw_bitmap_in_rect(ctx, battery_charging_bmp_white, image_rect);
						graphics_context_set_compositing_mode(ctx, GCompOpClear);
						graphics_draw_bitmap_in_rect(ctx, battery_charging_bmp_black, image_rect);
				}
		}
}

void handle_battery(BatteryChargeState charge_state)
{
		layer_mark_dirty(battery_layer);
}

void no_phone_update_callback(Layer *layer, GContext *ctx)
{
		if (config_show_no_phone && !bluetooth_connection_service_peek())
		{
				GRect image_rect = no_phone_bmp_white->bounds;
				graphics_context_set_compositing_mode(ctx, GCompOpOr);
				graphics_draw_bitmap_in_rect(ctx, no_phone_bmp_white, image_rect);
				graphics_context_set_compositing_mode(ctx, GCompOpClear);
				graphics_draw_bitmap_in_rect(ctx, no_phone_bmp_black, image_rect);
		}
}

void bluetooth_connection_callback(bool connected)
{
		layer_mark_dirty(no_phone_layer);
		if (config_vibe && !connected) {		
				static const uint32_t const segments[] = { 100, 200, 100, 200, 100 };
				VibePattern pat = {
						.durations = segments,
						.num_segments = ARRAY_LENGTH(segments),
				};				
				vibes_enqueue_custom_pattern(pat);
		}
}

static void update_config()
{
	bitmap_layer_set_bitmap(bg_layer, config_simple ? bg_bmp_simple : bg_bmp);
	layer_set_hidden((Layer*)second_hand_layer_black, !config_seconds);
	layer_set_hidden((Layer*)second_hand_layer_white, !config_seconds);
	tick_timer_service_unsubscribe();
	tick_timer_service_subscribe(config_seconds ? SECOND_UNIT : MINUTE_UNIT, handle_tick);
}

void in_received_handler(DictionaryIterator *received, void *context) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received config");
		Tuple *tuple = dict_find(received, MSG_SHOW_NO_PHONE);
		if (tuple) {
				config_show_no_phone = (strcmp(tuple->value->cstring, "true") == 0);
				layer_mark_dirty(no_phone_layer);
		}
		tuple = dict_find(received, MSG_SHOW_BATTERY);
		if (tuple) {
				config_show_battery = (strcmp(tuple->value->cstring, "true") == 0);
				layer_mark_dirty(battery_layer);
		}
		tuple = dict_find(received, MSG_VIBE);
		if (tuple) {
				config_vibe = (strcmp(tuple->value->cstring, "true") == 0);
		}
		tuple = dict_find(received, MSG_SIMPLE);
		if (tuple) {
				config_simple = (strcmp(tuple->value->cstring, "true") == 0);
		}
		tuple = dict_find(received, MSG_SECONDS);
		if (tuple) {
				config_seconds = (strcmp(tuple->value->cstring, "true") == 0);
		}
		update_config();
		persist_write_bool(MSG_SHOW_NO_PHONE, config_show_no_phone);
		persist_write_bool(MSG_SHOW_BATTERY, config_show_battery);
		persist_write_bool(MSG_VIBE, config_vibe);
		persist_write_bool(MSG_SIMPLE, config_simple);
		persist_write_bool(MSG_SECONDS, config_seconds);
}

// Handle the start-up of the app
static void handle_init() {
	if (persist_exists(MSG_SHOW_NO_PHONE))
		config_show_no_phone = persist_read_bool(MSG_SHOW_NO_PHONE);
	if (persist_exists(MSG_SHOW_BATTERY))
		config_show_battery = persist_read_bool(MSG_SHOW_BATTERY);
	if (persist_exists(MSG_VIBE))
		config_vibe = persist_read_bool(MSG_VIBE);
	if (persist_exists(MSG_SIMPLE))
		config_simple = persist_read_bool(MSG_SIMPLE);
	if (persist_exists(MSG_SECONDS))
		config_seconds = persist_read_bool(MSG_SECONDS);

	app_message_register_inbox_received(in_received_handler);
	app_message_open(64, 64);

	hour_bmp_black = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HOUR_HAND_BLACK);
	min_bmp_black = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MINUTE_HAND_BLACK);
	sec_bmp_black = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SECOND_HAND_BLACK);
	battery_bmp_black = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_BLACK);
	battery_charging_bmp_black = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_CHARGING_BLACK);
	no_phone_bmp_black = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NO_PHONE_BLACK);
	hour_bmp_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HOUR_HAND_WHITE);
	min_bmp_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MINUTE_HAND_WHITE);
	sec_bmp_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SECOND_HAND_WHITE);
	battery_bmp_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_WHITE);
	battery_charging_bmp_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_CHARGING_WHITE);
	no_phone_bmp_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NO_PHONE_WHITE);
	bg_bmp = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
	bg_bmp_simple = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_SIMPLE);

  window = window_create();

	bg_layer = bitmap_layer_create(GRect(0,0, 144, 168));
	bitmap_layer_set_bitmap(bg_layer, bg_bmp_simple);

	hour_hand_layer_black = rot_bitmap_layer_create(hour_bmp_black);
	minute_hand_layer_black = rot_bitmap_layer_create(min_bmp_black);
	second_hand_layer_black = rot_bitmap_layer_create(sec_bmp_black);
	rot_bitmap_set_compositing_mode(hour_hand_layer_black, GCompOpClear);
	rot_bitmap_set_compositing_mode(minute_hand_layer_black, GCompOpClear);
	rot_bitmap_set_compositing_mode(second_hand_layer_black, GCompOpClear);

	hour_hand_layer_white = rot_bitmap_layer_create(hour_bmp_white);
	minute_hand_layer_white = rot_bitmap_layer_create(min_bmp_white);
	second_hand_layer_white = rot_bitmap_layer_create(sec_bmp_white);
	rot_bitmap_set_compositing_mode(hour_hand_layer_white, GCompOpOr);
	rot_bitmap_set_compositing_mode(minute_hand_layer_white, GCompOpOr);
	rot_bitmap_set_compositing_mode(second_hand_layer_white, GCompOpOr);

	rot_bitmap_set_src_ic(hour_hand_layer_black, GPoint(33, 40));
	rot_bitmap_set_src_ic(minute_hand_layer_black, GPoint(16, 60));
	rot_bitmap_set_src_ic(second_hand_layer_black, GPoint(7, 44));
	rot_bitmap_set_src_ic(hour_hand_layer_white, GPoint(33, 40));
	rot_bitmap_set_src_ic(minute_hand_layer_white, GPoint(16, 60));
	rot_bitmap_set_src_ic(second_hand_layer_white, GPoint(7, 44));

	GRect frame = layer_get_frame((Layer*)hour_hand_layer_black);
	frame.origin.x = 144/2 - frame.size.w/2;
	frame.origin.y = 168/2 - frame.size.h/2;
	layer_set_frame((Layer*)hour_hand_layer_black, frame);
	layer_set_frame((Layer*)hour_hand_layer_white, frame);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "H X: %d, X: %d, W: %d, H: %d", frame.origin.x, frame.origin.y, frame.size.w, frame.size.h);

	frame = layer_get_frame((Layer*)minute_hand_layer_black);
	frame.origin.x = 144/2 - frame.size.w/2;
	frame.origin.y = 168/2 - frame.size.h/2;
	layer_set_frame((Layer*)minute_hand_layer_black, frame);
	layer_set_frame((Layer*)minute_hand_layer_white, frame);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "M X: %d, X: %d, W: %d, H: %d", frame.origin.x, frame.origin.y, frame.size.w, frame.size.h);

	frame = layer_get_frame((Layer*)second_hand_layer_black);
	frame.origin.x = 144/2 - frame.size.w/2;
	frame.origin.y = 168/2 - frame.size.h/2;
	layer_set_frame((Layer*)second_hand_layer_black, frame);
	layer_set_frame((Layer*)second_hand_layer_white, frame);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "S X: %d, X: %d, W: %d, H: %d", frame.origin.x, frame.origin.y, frame.size.w, frame.size.h);
	
	battery_layer = layer_create(GRect(120, 5, 20, 14));
	no_phone_layer = layer_create(GRect(7, 5, 11, 12));
	
	Layer *window_layer = window_get_root_layer(window);
	layer_add_child(window_layer, (Layer*)bg_layer);
	layer_add_child(window_layer, (Layer*)hour_hand_layer_black);
	layer_add_child(window_layer, (Layer*)hour_hand_layer_white);
	layer_add_child(window_layer, (Layer*)minute_hand_layer_black);
	layer_add_child(window_layer, (Layer*)minute_hand_layer_white);
	layer_add_child(window_layer, (Layer*)second_hand_layer_black);
	layer_add_child(window_layer, (Layer*)second_hand_layer_white);
	layer_add_child(window_layer, battery_layer);
	layer_add_child(window_layer, no_phone_layer);
	layer_set_update_proc(battery_layer, &battery_update_callback);
	layer_set_update_proc(no_phone_layer, &no_phone_update_callback);

	update_config();

	update_time();

	window_stack_push(window, true /* Animated */);	
	battery_state_service_subscribe(handle_battery);
	bluetooth_connection_service_subscribe(bluetooth_connection_callback);
}

static void handle_deinit() {
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	app_message_deregister_callbacks();
	rot_bitmap_layer_destroy(hour_hand_layer_black);
	rot_bitmap_layer_destroy(minute_hand_layer_black);
	rot_bitmap_layer_destroy(second_hand_layer_black);
	rot_bitmap_layer_destroy(hour_hand_layer_white);
	rot_bitmap_layer_destroy(minute_hand_layer_white);
	rot_bitmap_layer_destroy(second_hand_layer_white);
	layer_destroy(battery_layer);
	layer_destroy(no_phone_layer);
	gbitmap_destroy(hour_bmp_black);
	gbitmap_destroy(min_bmp_black);
	gbitmap_destroy(sec_bmp_black);
	gbitmap_destroy(hour_bmp_white);
	gbitmap_destroy(min_bmp_white);
	gbitmap_destroy(sec_bmp_white);
	gbitmap_destroy(bg_bmp);
	gbitmap_destroy(bg_bmp_simple);
	gbitmap_destroy(battery_bmp_black);
	gbitmap_destroy(battery_bmp_white);
	gbitmap_destroy(battery_charging_bmp_black);
	gbitmap_destroy(battery_charging_bmp_white);
	gbitmap_destroy(no_phone_bmp_black);
	gbitmap_destroy(no_phone_bmp_white);
}

int main(void)
{
	handle_init();
	app_event_loop();
	handle_deinit();
}
