#include <allegro5/allegro5.h>

int game(int argc, char **argv );

int game(int argc, char **argv )
{
    al_init();
    auto display = al_create_display(0, 0);
    auto queue = al_create_event_queue();
    auto timer = al_create_timer(1 / 60.0);
    auto redraw = true;
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_start_timer(timer);
    while (true) {
        if (redraw) {
            al_clear_to_color(al_map_rgb_f(1, al_get_time() - (int)(al_get_time()), 0));
            al_flip_display();
            redraw = false;
        }
        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);
        if (event.type == ALLEGRO_EVENT_TIMER) {
            redraw = true;
        }
    }
    return 0;
}
