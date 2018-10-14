
/* **********************************************************************
   * This is the header file containing the prototypes for the          *
   * Universal Mayhem functions.                                        *
   *                                                                    *
   ********************************************************************** */

#include "stdarg.h"

void perform_name(void);
void perform_nav(void);
void perform_comets(void);
void create_comets_file(void);
void plug_close_objects(char test_ion);
void plug_ship_close(char test_ion);
void check_sensor_enemy(char test_ion);
void plug_base_close(void);
void plug_close_comets(void);
void plug_remotes_close(void);
void plug_close_scouts(void);
void plug_close_swarms(void);
unsigned short swarm_sensor_scan(long thex, long they);
void perform_sell(void);
void perform_buy(void);
unsigned short have_enough(void);
unsigned short buy_remote(void);
unsigned char sell_remote(void);
unsigned short buy_power(void);
unsigned char sell_power(void);
unsigned short buy_cargo(void);
unsigned char sell_cargo(void);
unsigned short buy_crew(void);
unsigned char sell_crew(void);
unsigned short buy_warp(void);
unsigned char sell_warp(void);
unsigned short buy_shuttle(void);
unsigned char sell_shuttle(void);
unsigned short buy_hull(void);
unsigned char sell_hull(void);
unsigned short buy_sensor(void);
unsigned char sell_sensor(void);
unsigned short buy_cloak(void);
unsigned char sell_cloak(void);
unsigned short buy_torp(void);
unsigned char sell_torp(void);
unsigned short buy_sled(void);
unsigned char sell_sled(void);
void technology_level_bad(void);
void perform_find(void);
void offer_planets_position(void);
void perform_go(void);
void perform_fire(void);
void phaser_ship(unsigned int the_rnd, long hit_value);
void phaser_base(unsigned int the_rnd, long int hit_value);
void call_the_cops(void);
void locate_the_cops(void);
void perform_peace(void);
void fire_at_friend(short attacked_ship, short friend_number);
void perform_plague(void);
void perform_power(void);
void perform_ping(void);
void perform_protect(void);
long arandom(long low, long high);
void perform_quit(short exit_value);
void remove_command_file(char *the_name, char *because_of);
void perform_remote(void);
void remote_from(unsigned short remote_number);
void place_remote(unsigned short remote_number);
void recall_remote(unsigned short remote_number);
void perform_rpt(void);
void check_after_rpt(void);
void perform_sled(void);
void swarm_attack(long thex, long they, short the_ship);
void fire_from_swarm(short the_ship, short t_loop, short on_ship);
void drop_sled_group(short the_ship, short t_loop);
void perform_scan(void);
void scan_elements(void);
void check_test_comet_scan(void);
void perform_spy(void);
void bad_spy(void);
void remove_destroyed_remote(short rem_num);
void perform_stand(char to_file);
void perform_stat(char from_sysop);
void perform_taxes(void);
void perform_sysop(void);
void remove_destroyed(void);
void remove_specified(void);
void move_something(void);
void allow_the_movement_of_this(short count);
void move_ship(short count);
void move_base(short count);
void display_information(void);
void perform_dos(void);
void expand_player_file(void);
unsigned short try_author_pass(void);
void perform_time(void);
void perform_lease_agreement(void);
void notice_test(char *the_string, short check_sum);
void remove_all_files(void);
void log_error(char error_number);
void log_entry(char log_type);
void create_galactic_police(short the_stat);
void perform_torp(void);
void hit_star(char the_velocity);
void hit_mine(char the_velocity);
void hit_hole(void);
void hit_planet(void);
void hit_enemy(int the_ship, int hit_value);
void fire_back_torp(short the_ship);
void fire_back_phaser(short the_ship, UL how_much);
void enemy_shield_down(short the_ship);
void perform_evasive(short the_ship, short the_times);
void hit_base(long hit_value);
void hit_remote(unsigned int rloop);
void hit_swarm(unsigned int rloop);
void hit_cop(void);
unsigned short hit_decoy(void);
void clever_remarks_four(void);
void clever_remarks_five(void);
void perform_command(void);
void plug_command_files(void);
void read_command(short count);
void append_linked_list(short count);
unsigned short isname(char *tpoint);
void real_time_operation(void);
void skim_command_file_queue(void);
void command_assist(void);
void command_defend(void);
void command_attack(void);
void command_run_from(void);
void command_decoy(void);
unsigned short bail_1(void);
unsigned short bail_2(void);
unsigned short bail_3(void);
unsigned short bail_4(void);
void perform_bail_out(void);
void remove_link(void);
unsigned short examine_command_data(void);
void cant_do_it(short first_one, short second_one);
char perform_warp(void);
void find_power_needed(short the_vel);
void hit_comet(short cnumber);
void dewarp_star(void);
void dewarp_mine(void);
void make_some_damage(short on_ship, short from_ship);
void bounce_off_object(void);
void destroy_user(void);
void dewarp_hole(void);
void we_docked(void);
void encountered_warp_field(void);
void check_attached_remotes(short oldx, short oldy, UC oldz);
void perform_base(void);
void to_base(void);
void to_ship(void);
unsigned long ask_move_value(void);
void ship_shield(void);
void base_shield(void);
void ship_credits(void);
void base_credits(void);
void ship_cargo(void);
void base_cargo(void);
void ship_crew(void);
void base_crew(void);
void ship_cloak(void);
void base_cloak(void);
void half_ship(void);
void half_base(void);
void disassemble_base(void);
void repair_base(void);
void perform_board(void);
void board_ship(void);
void board_base(void);
void perform_leave(void);
void perform_info(long thex, long they);
void perform_help(void);
void perform_free(void);
void read_user(void);
void read_user(void);
void read_universe(unsigned int uni_record);
void read_enemy(unsigned int the_record);
void write_enemy(unsigned int the_record);
short find_specific_planet(long from_record, long specific_record);
void read_planets(long in_it);
void write_planets(long in_it);
void write_comets(void);
void plug_comets_values(void);
void perform_extended(char use_number, char item_number);
void dump_extended_help(FILE **help_file);
char distribute_command(void);
void perform_destruct(void);
void is_boarding(short the_ship);
void check_enemy_strength(short the_ship);
void perform_long(void);
void found_this_ship(short target);
void perform_msg(void);
void message_to_this(short count, char all_or_good_name, char *use_this);
void plug_msg(char *the_buffer, int char_count);
void msg_zeros(short acount);
void msg_out(char byte);
unsigned short make_new_message(char statistics_message);
int get_next_message(void);
void open_mail_box(void);
void open_message_file(void);
void timed_input(short echo_or_not);
void perform_auction(void);
void already_bidding(void);
void was_highest_bid(void);
void perform_names(void);
void strip_process_command(void);
short match(void);
int extract_config(short element_number);
void cfg_string(short element_number);
void config_bad(short element_number);
void make_zero_record(void);
void create_ship_file(void);
void plug_ship_values(void);
void plug_this_ship(char count, char allocate_memory);
void ucase(char *this_record);
void make_new_ship(void);
void check_letters(void);
void get_update_date(void);
void ship_destroyed(void);
void clever_remarks_one(void);
void clever_remarks_two(void);
void clever_remarks_three(void);
void plug_ion_fields(void);
void trigger_infection(void);
void impound_ship(void);
void process_origination_network_address(void);
void process_destination_network_address(void);
short find_delimiter(char delimit);
void perform_password(void);
unsigned char check_mutual_alliance(short the_ship, short on_ship);
unsigned char check_command_file(void);
unsigned char check_other_side(short the_ship, short on_ship);
void write_user(void);
char check_sysop_key(void);
short check_friend_base(short ships_number);
short is_a_friend(short ships_number);
short is_a_mutual_friend(short ships_number);
int rpt_check_com_port_for_key(void);
void edit_player_file(void);
void edit_this_file(short the_ship);
void change_power(void);
void change_credits(void);
void change_time(void);
void change_taxes(void);
char check_carrier_detect(char direction_test);
void lost_carrier(void);
unsigned short test_taxes_due(void);
short hand_over_planets(short from_this, short to_this);
void clear_ownership(void);
void perform_color(void);
void c_out(int the_color, va_list format, ...);
void return_phaser(short the_ship);
void return_torp(short the_ship);
void sick_bay(void);
void cops_buy_things_too(float the_amount_of_taxes);
void move_the_cops_around(void);
void pirates_fight_back(char first_blood);
void perform_bounty(void);
void create_goal_file(void);
void plug_goal_data(void);
void remove_goal_data(void);
void dewarp_slaver_device(short afloop);
void return_slaver_parts(short from_this_ship, char because_of_status);
void plug_close_goals(void);
void total_up_slaver_parts(void);
void perform_trigger(void);
void trigger_slaver_death_weapon(void);
void bounce_run_from_against_standings(void);
FILE *mayhem_fopen(char *fname, char *types, FILE *fpoint);
int mayhem_fclose(FILE **fname);
int mayhem_fcloseall(void);
int mayhem_fread(void *inbuf, size_t tsize, size_t tcount, FILE *fname);
int mayhem_fwrite(void *outbuf, size_t tsize, size_t tcount, FILE *fname);
void perform_universe_map(void);
void perform_universe_map(void);
char *what_universe_name(char this_universe);
void perform_leash(void);
void perform_scout(void);
char command_file_exist(char for_this_ship);
void memory_allocated(UL this_amount);
void memory_freed(UL this_amount);
UL memory_used(void);
void plug_scout_information(void);
void automate_scout_ships(void);
void send_character(char this_byte);
char return_character(void);
void send_string(char *this_string);
void ship_attacking(char the_ship);
void perform_talk(void);
void perform_chat(void);
void mail_check_statistics_package(void);
char is_it_a_good_name(char *this_string);
void import_statistics_file(void);
void inform_kill(char *killer, char *killed, char killed_a_ship, char how);
char assisting_ship(short this_ship);
void extend_question(char this_element);
void interrupt new_clock_tick(void);
void io_init(void);
void io_close(void);
void mail_slaver(char piece);
void perform_own(void);
char perform_macro(void);
void throw_com_port_away(void);
char extract_drop_information(void);
void test_morale_level(void);
void get_ready_to_leave(void);
void if_any_bounce_it(int top_count);

void mail_leash(short enemy_number,
    long fromx,
    long fromy,
    long tox,
    long toy,
    char how);

void swarm_on_automated(short the_ship,
    short t_loop,
    short on_ship,
    long hit_value);

short warp_ship(short ship_number,
    short the_dir,
    short the_vel,
    short priority,
    short the_limit);

short compute_direction(short ship_number,
    short user_number,
    short x_bounds,
    short y_bounds);

void bring_along_remote(short test_ship,
    short the_remote,
    long newx,
    long newy);

void mail_board(short enemy_number,
   char boarded_ship,
   UL took_power,
   UL took_credits,
   UL took_cargo,
   UL took_torps,
   char friendly);


