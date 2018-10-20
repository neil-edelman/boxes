/** Ship is a superclass of two different ships.

 @title Ship
 @author Neil
 @std C89 */

#include <stdio.h>	/* *printf */
#include <assert.h>	/* assert */
#include <string.h>	/* strncpy */
#include "Orcish.h"
#include "Ship.h"

/*struct Ship;*/
typedef void (*ShipToString)(const struct Ship *const, char (*const)[12]);
typedef void (*ShipAction)(struct Ship *const);
typedef void (*ShipsAction)(struct Ships *const, struct Ship *const);

/** Virtual {ShipVt} \in {Ship} as {*vt}. */
struct ShipVt {
	const char type[16];
	const ShipToString to_string;
	const ShipsAction delete;
};
/** Abstract {Ship}. */
struct Ship {
	const struct ShipVt *vt;
	char name[8];
};
/*static const size_t ship_name_size = sizeof ((struct Ship *)0)->name
	/ sizeof *((struct Ship *)0)->name;*/
/** @implements <Ship>ToString */
static void ship_to_string(const struct Ship *const ship, char (*const a)[12]) {
	assert(ship && a);
	ship->vt->to_string(ship, a);
}
#define LIST_NAME Ship
#define LIST_TYPE struct Ship
#define LIST_TO_STRING &ship_to_string
#include "List.h"

/* {Destroyer} extends {Ship}. */
struct Destroyer {
	struct ShipLink ship;
	unsigned no;
};
/** {container_of}. */
static struct Destroyer *destroyer_holds_ship(struct Ship *const ship) {
	return (struct Destroyer *)(void *)((char *)ship
		- offsetof(struct Destroyer, ship) - offsetof(struct ShipLink, data));
}
/** {const container_of}. */
static const struct Destroyer *
	const_destroyer_holds_ship(const struct Ship *const ship) {
	return (const struct Destroyer *)(const void *)((const char *)ship
		- offsetof(struct Destroyer, ship)
		- offsetof(struct ShipLink, data));
}
/** @implements <Ship>ToString */
static void destroyer_to_string(const struct Ship *const ship,
	char (*const a)[12]) {
	assert(ship && a);
	sprintf(*a, "%.9s%u", ship->name, const_destroyer_holds_ship(ship)->no%100);
	/*strncpy(*a, ship->name, sizeof *a / sizeof **a);*/
}
/** @implements <Destroyer>Migrate */
static void destroyer_migrate(struct Destroyer *const this,
	const struct Migrate *const migrate) {
	assert(this && migrate);
	ShipLinkMigrate(&this->ship.data, migrate);
}
#define POOL_NAME Destroyer
#define POOL_TYPE struct Destroyer
#define POOL_MIGRATE_EACH &destroyer_migrate
#include "Pool.h"

/* {Cruiser} extends {Ship}. */
struct Cruiser {
	struct ShipLink ship;
	char no;
};
/** {container_of}. */
static struct Cruiser *cruiser_holds_ship(struct Ship *const ship) {
	return (struct Cruiser *)(void *)((char *)ship
		- offsetof(struct Cruiser, ship) - offsetof(struct ShipLink, data));
}
/** {const container_of}. */
static const struct Cruiser *
	const_cruiser_holds_ship(const struct Ship *const ship) {
	return (const struct Cruiser *)(const void *)((const char *)ship
		- offsetof(struct Cruiser, ship) - offsetof(struct ShipLink, data));
}
/** @implements <Ship>ToString */
static void cruiser_to_string(const struct Ship *const ship,
	char (*const a)[12]) {
	assert(ship && a);
	sprintf(*a, "%.10s%c", ship->name, const_cruiser_holds_ship(ship)->no);
}
/** @implements <Cruiser>Migrate */
static void cruiser_migrate(struct Cruiser *const this,
	const struct Migrate *const migrate) {
	assert(this && migrate);
	ShipLinkMigrate(&this->ship.data, migrate);
}
#define POOL_NAME Cruiser
#define POOL_TYPE struct Cruiser
#define POOL_MIGRATE_EACH &cruiser_migrate
#include "Pool.h"

/** {Ship} list with backing. */
struct Ships {
	struct ShipList *grid;
	unsigned x, y;
	struct DestroyerPool destroyers;
	struct CruiserPool cruisers;
};
/** @implements <Ships,Ship>Action */
/*static void ship_delete(struct Ships *const ships, struct Ship *const ship) {
	assert(ships && ship);
	ShipListRemove(ship);
	ship->vt->delete(ships, ship);
}*/
/** Called from \see{ship_delete}.
 @implements <Ships,Ship>Action */
static void destroyer_delete(struct Ships *const ships, struct Ship *const ship)
{
	DestroyerPoolRemove(&ships->destroyers, destroyer_holds_ship(ship));
}
/** Called from \see{ship_delete}.
 @implements <Ships,Ship>Action */
static void cruiser_delete(struct Ships *const ships, struct Ship *const ship) {
	CruiserPoolRemove(&ships->cruisers, cruiser_holds_ship(ship));
}

/* Static data containing the virtual functions defined above. */
static struct ShipVt destroyer_vt
	= { "Destroyer", &destroyer_to_string, &destroyer_delete };
static struct ShipVt cruiser_vt
	= { "Criuser", &cruiser_to_string, &cruiser_delete };

/** Only called from constructors of children; if you will, abstract. */
static void Ship(struct Ship *const ship, const struct ShipVt *const vt) {
	assert(ship && vt);
	ship->vt = vt;
	Orcish(ship->name, sizeof ship->name);
}

static void grid_clear(struct Ships *const s) {
	unsigned x, y;
	const unsigned x_max = s->x, y_max = s->y;
	assert(s);
	for(y = 0; y < y_max; y++) {
		for(x = 0; x < x_max; x++) {
			ShipListClear(&s->grid[x + y * x_max]);
		}
	}
}

/** Destructor for {Ships}. */
void Ships_(struct Ships **const pships) {
	struct Ships *ships;
	if(!pships || !(ships = *pships)) return;
	grid_clear(ships);
	DestroyerPool_(&ships->destroyers);
	CruiserPool_(&ships->cruisers);
	free(ships), *pships = ships = 0;
}
/** Constructor.
 @throws {malloc}
 @throws EDOM: If {x} or {y} is zero or {x * y} overflows a {size_t}. */
struct Ships *Ships(const unsigned x, const unsigned y) {
	struct Ships *ships;
	if(!x || !y /* @fixme And . . . */) { errno = EDOM; return 0; }
	if(!(ships = malloc(sizeof *ships + x * y * sizeof *ships->grid))) return 0;
	ships->grid = (struct ShipList *)(ships + 1);
	ships->x = x, ships->y = y;
	DestroyerPool(&ships->destroyers);
	CruiserPool(&ships->cruisers);
	grid_clear(ships);
	return ships;
}
/** Put it in a random position. Must not be assigned a position. Must be in
 the same object. */
static void random_list(struct Ships *const ships, struct Ship *const ship) {
	size_t idx;
	assert(ships && ship);
	idx = ((float)ships->y * ships->x * rand()) / RAND_MAX;
	assert(idx < ships->x * ships->y);
	ShipListPush(ships->grid + idx, ship);
}
/** Constructor for a {Destroyer} in {ships}. */
struct Destroyer *Destroyer(struct Ships *const ships) {
	struct Destroyer *destroyer;
	if(!ships || !(destroyer = DestroyerPoolNew(&ships->destroyers))) return 0;
	Ship(&destroyer->ship.data, &destroyer_vt);
	destroyer->no = (int)(89.0 * rand() / RAND_MAX) + 10;
	random_list(ships, &destroyer->ship.data);
	return destroyer;
}
/** Constructor for a {Cruiser} in {ships}. */
struct Cruiser *Cruiser(struct Ships *const ships) {
	struct Cruiser *cruiser;
	if(!ships || !(cruiser = CruiserPoolNew(&ships->cruisers))) return 0;
	Ship(&cruiser->ship.data, &cruiser_vt);
	cruiser->no = 'A' + (char)(26.0 * rand() / RAND_MAX);
	random_list(ships, &cruiser->ship.data);
	return cruiser;
}
/** Clears all the {ships}. */
void ShipsClear(struct Ships *const ships) {
	if(!ships) return;
	grid_clear(ships);
	DestroyerPoolClear(&ships->destroyers);
	CruiserPoolClear(&ships->cruisers);
}
void ShipsOut(const struct Ships *const ships) {
	unsigned x, y;
	const unsigned x_max = ships->x, y_max = ships->y;
	if(!ships) return;
	for(y = 0; y < y_max; y++) {
		for(x = 0; x < x_max; x++) {
			printf("%s%s", ShipListToString(ships->grid + x + y * x_max),
				x == x_max - 1 ? "\n" : ", ");
		}
	}
}
