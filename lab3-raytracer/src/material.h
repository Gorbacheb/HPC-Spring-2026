#pragma once

#include "vector.h"

/**
 * @brief      Материал поверхности для модели освещения Фонга.
 *
 * @details
 *             Соответствует стандарту Wavefront .mtl (Material Template Library):
 *             @see https://en.wikipedia.org/wiki/Wavefront_.obj_file#Material_template_library
 */
struct Material {
    /**
     * @brief   Цвет фонового освещения.
     * @details Имитирует рассеянный свет, падающий на объект со всех сторон.
     *          В .mtl соответствует ключу `Ka`.
     * @see https://en.wikipedia.org/wiki/Phong_reflection_model
     */
    Vector ambient_color = Vector(0.0, 0.0, 0.0);

    /**
     * @brief   Цвет рассеянного освещения.
     * @details Основной цвет материала. Интенсивность зависит от угла падения света.
     *          В .mtl соответствует ключу `Kd`.
     @see https://en.wikipedia.org/wiki/Diffuse_reflection
     */
    Vector diffuse_color = Vector(0.0, 0.0, 0.0);

    /**
     * @brief   Цвет зеркальных бликов.
     * @details Цвет отраженного света в бликах.
     *          В .mtl соответствует ключу `Ks`.
     * @see https://en.wikipedia.org/wiki/Specular_highlight
     */
    Vector specular_color = Vector(0.0, 0.0, 0.0);

    /**
     * @brief   Интенсивность собственного свечения.
     * @details Материал испускает свет, не зависящий от внешних источников.
     *          В .mtl соответствует ключу `Ke`.
     */
    Vector intensity = Vector(0.0, 0.0, 0.0);

    /**
     * @brief   Альбедо материала (коэффициент отражения).
     * @details Определяет долю рассеянно отраженного света.
     *          Красный вектор (1,0,0) в примере означает полное отражение красной компоненты.
     */
    Vector albedo = Vector(1.0, 0.0, 0.0);

    /**
     * @brief   Степень зеркального блеска.
     * @details Чем выше значение, тем острее и ярче блик, тем глаже кажется поверхность.
     *          В .mtl соответствует ключу `Ns`.
     */
    double specular_exponent = 1.0;

    /**
     * @brief   Показатель преломления.
     * @details Влияет на траекторию преломленных лучей (используется для прозрачных материалов).
     *          В .mtl соответствует ключу `Ni`.
     * @see https://en.wikipedia.org/wiki/Refractive_index
     */
    double refraction_index = 1.0;
};
