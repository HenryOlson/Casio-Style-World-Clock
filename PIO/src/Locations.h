#define LOCAL_INDEX 0
#define GMT_INDEX 1
#define MAX_LOCATIONS 40

#ifndef LOCATIONS_H
#define LOCATIONS_H
struct Location
{
    const char *tz;
    const char *locationCode;
    int locX;
    int locY;
};
class Locations {
    private:
    // ToDo: NB: if this is private, locations is not properly initialized
    //Location locations[MAX_LOCATIONS];
    int locationCount;
    public:
    // lifecycle
    void localize();
    // accessors
    Location* getLocation(int i);
    int ixByName(char* name);
    // helper
    int nextLocation(int current);
};
#endif

