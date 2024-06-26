
struct Ray
{
    int depth; //times a ray has been shot
    int done; //if = 1 ray will not reshoot or else it will repeat (for reflections)
    vec3 hitValue; //color gained from shoot
    float distanceTraveled;
    vec3 rayOrigin; 
    vec3 rayDir;
    int targetTLAS;
};