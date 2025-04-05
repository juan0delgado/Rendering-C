#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "EDM.h"
#include <sys/stat.h>
#define SIDE 512
#define SCALE 2048
#define SKY 1e+30
#define TOTAL_FRAMES 120

typedef struct {
    VEC3_T o, d;
} RAY_T;

typedef struct {
    VEC3_T c;
    float r;
} SPHERE_T;

typedef struct {
    VEC3_T base;
    VEC3_T axis;
    float radius;
    float height;
} CYLINDER_T;

typedef struct {
    VEC3_T apex;
    VEC3_T axis;
    float radius;
    float height;
} CONE_T;

VEC3_T get_light_position(VEC3_T camera_position) {
    // Ajusta estos valores para cambiar el ángulo de iluminación
    VEC3_T light_offset = {200.0, 500.0, -200.0}; // Posición de la luz 200.0, 300.0, -200.0
    return VEC3_add(camera_position, light_offset);
}

void create_directory_if_not_exists(const char *dir) {
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        mkdir(dir, 0700);  // Crear directorio con permisos 0700 si no existe
    }
}

float Shading(float dst, RAY_T ray, VEC3_T normal, VEC3_T light, int objectType) {
    if (dst == SKY) return 0.0;
    float ambient = 0.0;
    VEC3_T normal_normalized = VEC3_normalize(normal);
    float diffuse = VEC3_dot(normal_normalized, light);
    diffuse = (diffuse < 0.0) ? 0.0 : diffuse;
    float intensity = (objectType == 2) ? 1.5 : 1.2; // intensidad de la luz  2.0 : 1.5
    return (ambient + diffuse * intensity);
}

void rotate_scene(float angle_deg, VEC3_T *light) {
    float angle = angle_deg * M_PI / 180.0;
    float cos_theta = cos(angle);
    float sin_theta = sin(angle);

    // Rotar la luz junto con la cámara
    float light_x = light->x;
    float light_z = light->z;
    light->x = light_x * cos_theta - light_z * sin_theta;
    light->z = light_x * sin_theta + light_z * cos_theta;
}

float SphereIntersection(RAY_T ray, SPHERE_T sphere) {
    VEC3_T oc = VEC3_sub(ray.o, sphere.c);
    float a = VEC3_dot(ray.d, ray.d);
    float b = 2.0 * VEC3_dot(oc, ray.d);
    float c = VEC3_dot(oc, oc) - sphere.r * sphere.r;
    float discriminant = b*b - 4*a*c;
    if (discriminant < 0) return SKY;

    float t1 = (-b - sqrt(discriminant)) / (2*a);
    if (t1 > 0) return t1;
    float t2 = (-b + sqrt(discriminant)) / (2*a);
    return (t2 > 0) ? t2 : SKY;
}
float CylinderIntersection(RAY_T ray, CYLINDER_T cylinder) {
    // Vector from ray origin to cylinder base
    VEC3_T oc = VEC3_sub(ray.o, cylinder.base);

    // Calculate projections
    float dot_d_axis = VEC3_dot(ray.d, cylinder.axis);
    float dot_oc_axis = VEC3_dot(oc, cylinder.axis);

    // Components perpendicular to cylinder axis
    VEC3_T d_perp = VEC3_sub(ray.d, VEC3_scale(cylinder.axis, dot_d_axis));
    VEC3_T oc_perp = VEC3_sub(oc, VEC3_scale(cylinder.axis, dot_oc_axis));

    // Quadratic equation coefficients for infinite cylinder
    float a = VEC3_dot(d_perp, d_perp);
    float b = 2.0f * VEC3_dot(d_perp, oc_perp);
    float c = VEC3_dot(oc_perp, oc_perp) - cylinder.radius * cylinder.radius;

    float discriminant = b*b - 4*a*c;

    // No intersection with infinite cylinder
    if (discriminant < 0) return SKY;

    // Calculate infinite cylinder intersections
    float t1 = (-b - sqrt(discriminant)) / (2*a);
    float t2 = (-b + sqrt(discriminant)) / (2*a);

    // Order intersections
    if (t1 > t2) {
        float temp = t1;
        t1 = t2;
        t2 = temp;
    }

    // Calculate height positions at intersections
    float h1 = dot_oc_axis + t1 * dot_d_axis;
    float h2 = dot_oc_axis + t2 * dot_d_axis;

    // Check if intersection points are within height limits
    bool t1_valid = (t1 > 0) && (h1 >= 0) && (h1 <= cylinder.height);
    bool t2_valid = (t2 > 0) && (h2 >= 0) && (h2 <= cylinder.height);

    // Check for cap intersections
    float tcap1 = SKY; // Bottom cap
    float tcap2 = SKY; // Top cap

    // Bottom cap (base)
    if (fabs(dot_d_axis) > 1e-6) {
        float t = -dot_oc_axis / dot_d_axis;
        if (t > 0) {
            VEC3_T p = VEC3_add(ray.o, VEC3_scale(ray.d, t));
            VEC3_T v = VEC3_sub(p, cylinder.base);
            float dist2 = VEC3_dot(v, v) - pow(VEC3_dot(v, cylinder.axis), 2);
            if (dist2 <= cylinder.radius * cylinder.radius) {
                tcap1 = t;
            }
        }
    }

    // Top cap
    VEC3_T top_center = VEC3_add(cylinder.base, VEC3_scale(cylinder.axis, cylinder.height));
    VEC3_T oc_top = VEC3_sub(ray.o, top_center);
    float dot_oc_top_axis = VEC3_dot(oc_top, cylinder.axis);

    if (fabs(dot_d_axis) > 1e-6) {
        float t = -dot_oc_top_axis / dot_d_axis;
        if (t > 0) {
            VEC3_T p = VEC3_add(ray.o, VEC3_scale(ray.d, t));
            VEC3_T v = VEC3_sub(p, top_center);
            float dist2 = VEC3_dot(v, v) - pow(VEC3_dot(v, cylinder.axis), 2);
            if (dist2 <= cylinder.radius * cylinder.radius) {
                tcap2 = t;
            }
        }
    }

    // Find closest valid intersection
    float t_min = SKY;

    if (t1_valid) t_min = t1;
    else if (t2_valid) t_min = t2;

    if (tcap1 < t_min) t_min = tcap1;
    if (tcap2 < t_min) t_min = tcap2;

    return t_min;
}

// Normal del cilindro (incluyendo tapas)
VEC3_T CylinderNormal(VEC3_T hit_point, CYLINDER_T cylinder) {
    // Vector to base
    VEC3_T vec_to_base = VEC3_sub(hit_point, cylinder.base);
    float dot_with_axis = VEC3_dot(vec_to_base, cylinder.axis);

    // Check bottom cap
    if (fabs(dot_with_axis) < 1e-4) {
        return VEC3_scale(cylinder.axis, -1.0f); // Normal apunta hacia abajo
    }

    // Check top cap
    VEC3_T top_center = VEC3_add(cylinder.base, VEC3_scale(cylinder.axis, cylinder.height));
    if (VEC3_lenght(VEC3_sub(hit_point, top_center)) < cylinder.radius + 1e-4) {
        return cylinder.axis; // Normal apunta hacia arriba
    }

    // Side normal
    VEC3_T proj_on_axis = VEC3_scale(cylinder.axis, dot_with_axis);
    return VEC3_normalize(VEC3_sub(vec_to_base, proj_on_axis));
}



float ConeIntersection(RAY_T ray, CONE_T cone) {
    VEC3_T base_center = VEC3_add(cone.apex, VEC3_scale(cone.axis, cone.height));
    VEC3_T oc = VEC3_sub(ray.o, cone.apex);

    float cos_half_angle = cone.height / sqrt(cone.height * cone.height + cone.radius * cone.radius);
    float sin_half_angle = cone.radius / sqrt(cone.height * cone.height + cone.radius * cone.radius);
    float cos2 = cos_half_angle * cos_half_angle;
    float sin2 = sin_half_angle * sin_half_angle;

    float dot_d_axis = VEC3_dot(ray.d, cone.axis);
    float dot_oc_axis = VEC3_dot(oc, cone.axis);

    float a = dot_d_axis * dot_d_axis - cos2 * VEC3_dot(ray.d, ray.d);
    float b = 2.0f * (dot_d_axis * dot_oc_axis - cos2 * VEC3_dot(ray.d, oc));
    float c = dot_oc_axis * dot_oc_axis - cos2 * VEC3_dot(oc, oc);

    float discriminant = b*b - 4*a*c;

    if (discriminant < 0) return SKY;

    float t1 = (-b - sqrt(discriminant)) / (2*a);
    float t2 = (-b + sqrt(discriminant)) / (2*a);

    if (t1 > t2) {
        float temp = t1;
        t1 = t2;
        t2 = temp;
    }

    float h1 = dot_oc_axis + t1 * dot_d_axis;
    float h2 = dot_oc_axis + t2 * dot_d_axis;

    bool t1_valid = (t1 > 0) && (h1 >= 0) && (h1 <= cone.height);
    bool t2_valid = (t2 > 0) && (h2 >= 0) && (h2 <= cone.height);

    if (t1_valid) return t1;
    if (t2_valid) return t2;
    return SKY;
}



VEC3_T ConeNormal(VEC3_T hit_point, CONE_T cone) {
    // Calcular el centro de la base del cono
    VEC3_T base_center = VEC3_add(cone.apex, VEC3_scale(cone.axis, cone.height));

    // Vector desde el vértice hasta el punto de intersección
    VEC3_T v = VEC3_sub(hit_point, cone.apex);
    float dot_v_axis = VEC3_dot(v, cone.axis);

    // Verificar si el punto está en la base
    if (fabs(dot_v_axis - cone.height) < 1e-4) {
        return cone.axis;  // La normal en la base apunta hacia el eje del cono
    }

    // Si el punto está en la parte lateral, calcular la normal
    VEC3_T axis_point = VEC3_add(cone.apex, VEC3_scale(cone.axis, dot_v_axis));
    VEC3_T normal = VEC3_sub(hit_point, axis_point);

    // Ajustar la normal para la pendiente del cono
    float cos_half_angle = cone.height / sqrt(cone.height * cone.height + cone.radius * cone.radius);
    VEC3_T adjusted_normal = VEC3_normalize(normal);
    float component = VEC3_dot(adjusted_normal, cone.axis) / cos_half_angle;
    VEC3_T parallel = VEC3_scale(cone.axis, component);
    VEC3_T perp = VEC3_sub(adjusted_normal, parallel);

    return VEC3_normalize(perp);
}
int main() {
    create_directory_if_not_exists("frames");

    // Configuración de la cámara (idéntica a tu versión)
    float camera_radius = 800.0;
    float camera_height = 300.0;

    for (int frame = 0; frame < TOTAL_FRAMES; ++frame) {
        char filename[64];
        sprintf(filename, "frames/frame_%03d.pgm", frame);
        FILE *fp = fopen(filename, "w");

        if (!fp) {
            perror("No se pudo crear el archivo");
            return 1;
        }

        // Figuras estáticas en el centro (mismos parámetros)
        SPHERE_T sphere = { {0.0, 0.0, 500.0}, 100.0 };
        CYLINDER_T cylinder = { {-200.0, -50.0, 500.0}, {0.0, 1.0, 0.0}, 60.0, 120.0 };
        CONE_T cone = { {200.0, 50.0, 500.0}, {0.0, -1.0, 0.0}, 60.0, 120.0 };

        // Ángulo de rotación (igual)
        float angle_deg = (360.0 * frame) / TOTAL_FRAMES;

        // Posición de la cámara (idéntica)
        VEC3_T camera_position = {
            camera_radius * cos(angle_deg * M_PI / 180.0),
            camera_height,
            camera_radius * sin(angle_deg * M_PI / 180.0) + 500.0
        };

        // Configuración de la LUZ (mismo sistema que tu versión)
        VEC3_T light_pos = get_light_position(camera_position);
        VEC3_T light = VEC3_normalize(VEC3_sub(light_pos, (VEC3_T){0,0,500}));

        // Sistema de coordenadas de la cámara (sin cambios)
        VEC3_T look_at = {0.0, 0.0, 500.0};
        VEC3_T up = {0.0, 1.0, 0.0};
        VEC3_T forward = VEC3_normalize(VEC3_sub(look_at, camera_position));
        VEC3_T right = VEC3_normalize(VEC3_cross(forward, up));
        VEC3_T real_up = VEC3_normalize(VEC3_cross(right, forward));

        fprintf(fp, "P2\n%d %d\n%d\n", SIDE, SIDE, SCALE);

        // Ray tracing (solo cambian las funciones de cilindro)
        for (int y = SIDE - 1; y >= 0; --y) {
            for (int x = 0; x < SIDE; ++x) {
                // Coordenadas normalizadas del píxel (igual)
                float px = (x - SIDE / 2.0) / (SIDE / 2.0);
                float py = (y - SIDE / 2.0) / (SIDE / 2.0);

                // Dirección del rayo (idéntico)
                VEC3_T ray_direction = VEC3_normalize(VEC3_add(
                    VEC3_add(VEC3_scale(right, px), VEC3_scale(real_up, py)),
                    VEC3_scale(forward, 1.0)
                ));

                RAY_T ray = { camera_position, ray_direction };

                // Intersecciones (usa las NUEVAS funciones para cilindro)
                float dist_sphere = SphereIntersection(ray, sphere);
                float dist_cylinder = CylinderIntersection(ray, cylinder);  // ¡Nueva versión con tapas!
                float dist_cone = ConeIntersection(ray, cone);

                // Encontrar el objeto más cercano (igual)
                float min_dist = fminf(fminf(dist_sphere, dist_cylinder), dist_cone);
                float diffuse = 0.0;

                if (min_dist == dist_sphere) {
                    VEC3_T hit = VEC3_add(ray.o, VEC3_scale(ray.d, min_dist));
                    VEC3_T normal = VEC3_normalize(VEC3_sub(hit, sphere.c));
                    diffuse = Shading(min_dist, ray, normal, light, 1);
                } 
                else if (min_dist == dist_cylinder) {
                    VEC3_T hit = VEC3_add(ray.o, VEC3_scale(ray.d, min_dist));
                    VEC3_T normal = CylinderNormal(hit, cylinder);  // ¡Nueva versión con normales de tapas!
                    diffuse = Shading(min_dist, ray, normal, light, 3);
                } 
                else if (min_dist == dist_cone) {
                    VEC3_T hit = VEC3_add(ray.o, VEC3_scale(ray.d, min_dist));
                    VEC3_T normal = ConeNormal(hit, cone);
                    diffuse = Shading(min_dist, ray, normal, light, 2);
                }

                // Luz ambiental + difusa (igual)
                diffuse = 0.1 + 0.9 * diffuse;
                int color = (int)(fminf(diffuse, 1.0) * SCALE);
                fprintf(fp, "%d ", color);
            }
            fprintf(fp, "\n");
        }
        fclose(fp);
    }
    return 0;
}
