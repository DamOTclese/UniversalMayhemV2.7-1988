
/* **********************************************************************
   * This is the structure of the SHIP.DAT data file.                   *
   *                                                                    *
   * As you can see, it contains all of the information needed for the	*
   * retention of all things the player has control/ownership over.     *
   *                                                                    *
   ********************************************************************** */

struct ship_file {          /* Structure of SHIP.DAT file                 */
  long ship_xpos;           /* The record number (xpos) of where ship is  */
  long ship_ypos;           /* The ypos of where the ship is              */
  UC ship_universe;         /* The universe the ship is in.               */
  UL ship_power;            /* The amount of power in ships engines       */
  long ship_shield;         /* The amount of power in ships shields       */
  UL ship_credits;          /* The amount of money in the ship            */
  short ship_cargo;         /* The amount of cargo in the ship            */
  short ship_crew;          /* The number of crew members in the ship     */
  short ship_shuttle;       /* The class shuttle craft in the ship        */
  short ship_hull;          /* The class hull the ship is                 */
  short ship_cloak;         /* The class cloaking device the ship has     */
  short ship_sensor;        /* The class sensor the ship has              */
  char ship_name[5];        /* The name of the ship everyone sees         */
  char ship_pass[21];       /* Password for getting into captains chair   */
  char ship_date[27];       /* Date last signed in                        */
  short ship_warp;          /* Class warp drive the ship has              */
  short ship_torp;          /* Number of torps the ship contains          */
  long base_xpos;           /* The record number (xpos) of ships base     */
  long base_ypos;           /* The ypos of the ships resupply base        */
  UC base_universe;         /* The universe the base is in                */
  UL base_credits;          /* The amount of money in the supply base     */
  UL base_cargo;            /* The amount of cargo in the supply base     */
  UL base_crew;             /* The number of crew members in base         */
  short base_cloak;         /* The class cloak of the base                */
  long base_shield;         /* The number of energy units in base shield  */
  char ship_person[31];     /* The personal name of the captain of ship   */
  short rem_xpos[10];       /* Record numbers (yposes) of remotes         */
  short rem_ypos[10];       /* xposes of four remote robot sensors        */
  UC rem_universe[10];      /* The universe the remote is in              */
  short total_kills;        /* The number of times this ship has killed   */
  short time_remaining;     /* Number of minutes remaining to play        */
  short planets_owned;      /* Number of planets captain has named        */
  short plague_flag;        /* 0 is no plague, else greater than 0.       */
  short ship_infected;      /* Is ship infected? Holds class infection    */
  short ship_morale;        /* Morale factor from 0% to 100%	     	  */
  char allies[5][5];        /* Up to 5 ships to be allied with names      */
  float taxes;	            /* Accumulative taxes for Galactic Police.    */
  char tax_warnings;        /* Number of warnings on taxes / Auction flg  */
  short attack_sleds;	    /* Number of attack sleds on board	     	  */
  short sled_xpos[15];      /* Xposition of up to 15 attack sleds         */
  short sled_ypos[15];      /* Yposition of up to 15 attack sleds         */
  UC sled_universe[15];     /* The universe the sled is in                */
  short sled_swarm[15];     /* Number of ships in up to 15 attack swarms  */
  short sled_power[15];	    /* Average power available for the swarms     */
  char last_at_by[5];	    /* Name of ship that last attacked this ship  */
  char last_at_who[5];      /* Name of ship this ship last attacked       */
  char last_at_status;      /* Status of the last ship attacked           */
  short last_torp_count;    /* number of torps by last attacked by        */
  float last_phaser_count;  /* Number of millions of units last atkd by   */
  short log_count;          /* Number of times captain has signed in      */
  char base_death[5];	    /* Who destroyed this ships base?	          */
  short base_hit_count;	    /* Number of times base hit while unshielded  */
  long x_planet_info[10];   /* X-Position of up to ten planets or points  */
  long y_planet_info[10];   /* Y-Position of up to ten planets or points  */
  char outstanding_bid;     /* Ships number bidded on                     */
  long bid_amount;          /* Amount of the bid                          */
  char base_boarded[5];     /* Allied ship that last boarded ships base   */
  float planet_taxes[10];   /* Taxes owed at ten planets.                 */
  long tax_xpos[10];	    /* Xpositions of the ten tax due planets      */
  long tax_ypos[10];	    /* Ypositions of the ten tax due planets	  */
  char slot_owned[10][5];   /* Name of the ship that owns taxed planet	  */
  short owned_planets[OWNABLE]; /* Xpos of 1 of 4 possible ypos of owned  */
  unsigned allow_colors : 1;/* Set to either 'Y' or 'N' for ansi colors   */
  short sick_bay;           /* Number of crew members in sick bay.        */
  long bounty;              /* Number of credits available for bounty     */
  char who_destroyed[5];    /* Name of ship that destroyed this ship.     */
  short scout_xpos[10];     /* Xpos of the scouts                         */
  short scout_ypos[10];     /* Ypos of the scouts                         */
  short scout_to_x[10];     /* Xpos of where to warp to                   */
  short scout_to_y[10];     /* Ypos of where to warp to                   */
  char scout_direction[10]; /* 0-Outbound, 1-Station keeping , 2-Inbound  */
  UC scout_universe[10];    /* What universe is the scout in?             */
  char stay_on_station[10]; /* Will it stay on station? No? It'll return. */
  char pirate_code[30];     /* The pirate access code                     */
  char leashed_by;          /* Ship this one is leased by                 */
  char leashed_to;          /* Ship this one has got a leash on           */
  char local;               /* 0 if local, 1 if imported                  */
  char point_tag[20][21];   /* Symbolic name of x/y posistion tag name.   */
  char macro[MACROS][201];  /* Keyboard Macros.                           */
} ;

extern struct ship_file *ships, *enemy;

