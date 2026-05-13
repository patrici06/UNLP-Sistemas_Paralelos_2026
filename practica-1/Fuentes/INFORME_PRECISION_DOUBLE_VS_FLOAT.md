# Informe: Análisis de Precisión entre `double` y `float` en C

## Resumen Ejecutivo

El presente informe analiza el comportamiento de los tipos de datos `float` y `double` en la resolución de ecuaciones cuadráticas, demostrando que la diferencia de precisión es directamente proporcional al tamaño en bytes y a la representación interna según el estándar IEEE 754.

---

## 1. Especificación de Tipos de Datos

### 1.1 Tamaño en Memoria

| Tipo de Dato | Tamaño | Bits | Mantisa | Exponente |
|--------------|--------|------|---------|-----------|
| `float`      | 4 bytes | 32   | 23 bits | 8 bits    |
| `double`     | 8 bytes | 64   | 52 bits | 11 bits   |

**Conclusión**: El `double` tiene el **doble de tamaño** que `float` (8 vs 4 bytes), lo que permite mayor precisión en la representación de números flotantes.

### 1.2 Estándar IEEE 754

Ambos tipos utilizan el estándar IEEE 754 para representación de números flotantes, pero con diferencias clave:

- **`float` (IEEE 754 - 32 bits)**:
  - Mantisa de 23 bits → precisión de ~7 dígitos decimales
  - Rango: ±3.4 × 10^38

- **`double` (IEEE 754 - 64 bits)**:
  - Mantisa de 52 bits → precisión de ~15-17 dígitos decimales
  - Rango: ±1.7 × 10^308

---

## 2. Código Analizado

```c
#define A 1.0
#define B -4.0000000
#define C 3.9999999

void dbl_solve(double a, double b, double c)
{
    double d = pow(b,2) - 4.0*a*c;
    double sd = sqrt(d);
    double r1 = (-b + sd) / (2.0*a);
    double r2 = (-b - sd) / (2.0*a);
    printf("Soluciones Double: %.5f\t%.5f\n", r1, r2);
}

void flt_solve(float a, float b, float c)
{
    float d = pow(b,2) - 4.0*a*c;
    float sd = sqrt(d);
    float r1 = (-b + sd) / (2.0*a);
    float r2 = (-b - sd) / (2.0*a);
    printf("Soluciones Float: %.5f\t%.5f\n", r1, r2);
}
```

### 2.1 Parámetros de la Ecuación Cuadrática (ax² + bx + c = 0)

- a = 1.0
- b = -4.0000000
- c = 3.9999999

### 2.2 Discriminante (Δ)

Δ = b² - 4ac = (-4)² - 4(1)(3.9999999) = 16 - 15.9999996 = 0.0000004

Este discriminante extremadamente pequeño es **crítico** para observar la diferencia de precisión.

---

## 3. Resultados Observados

### 3.1 Ejecución del Programa

```
Soluciones Float: 2.00000    2.00000
Soluciones Double: 2.00032   1.99968
```

### 3.2 Interpretación de Resultados

**Con `float`:**
- r1 = 2.00000 (redondeado)
- r2 = 2.00000 (redondeado)
- **Problema**: Se pierden ambas raíces con los dígitos significativos

**Con `double`:**
- r1 = 2.00032
- r2 = 1.99968
- **Ventaja**: Captura las diferencias pequeñas pero significativas

### 3.3 Valores Teóricos Exactos

Usando la fórmula general: x = (-b ± √Δ) / (2a)

x = (4 ± √0.0000004) / 2 = (4 ± 0.000632456) / 2

- **r1 = (4 + 0.000632456) / 2 = 2.000316228**
- **r2 = (4 - 0.000632456) / 2 = 1.999683772**

---

## 4. Análisis de la Pérdida de Precisión en `float`

### 4.1 Causa Raíz

La pérdida de precisión en `float` ocurre en múltiples etapas:

1. **Representación del discriminante**:
   - El valor 0.0000004 es muy pequeño
   - `float` (23 bits de mantisa) no puede representar esta magnitud con precisión
   - `double` (52 bits de mantisa) lo representa correctamente

2. **Cálculo de la raíz cuadrada**:
   - `sqrt()` retorna `double`, pero al asignar a `float`, hay conversión con pérdida
   - El valor √0.0000004 ≈ 0.000632... se redondea excesivamente en `float`

3. **Operación final**:
   - r1 = (-b + sd) / 2.0 pierde los dígitos significativos al trabajar con `float`

### 4.2 Visualización del Redondeo

```
Valor teórico:     r1 = 2.000316228
Representado en `double`:  2.00032  (captura 5 dígitos decimales)
Representado en `float`:   2.00000  (redondea todos los decimales)

Diferencia de precisión:
- double puede almacenar ~15 dígitos significativos
- float puede almacenar ~7 dígitos significativos
```

---

## 5. Conclusiones

### 5.1 Correctitud del Análisis Original

Tu análisis es **correcto y bien fundamentado**:

✅ "El `double` es más PRECISO que el `float`"
- Confirmado por estándar IEEE 754 y resultados empíricos

✅ "`double` tiene 8 bytes, `float` tiene 4 bytes"
- Confirmado: relación 2:1 en tamaño de memoria

✅ "`float` redondea a 2.0000"
- Confirmado: ambas raíces se redondean a 2.00000

✅ "Los resultados teóricos son r1 = 2.000316228 y r2 = 1.99968"
- Confirmado: `double` captura valores muy cercanos (2.00032 y 1.99968)

### 5.2 Razones de la Diferencia de Precisión

| Factor | Explicación |
|--------|------------|
| **Tamaño en bytes** | 8 bytes vs 4 bytes = más bits para representación |
| **Bits de mantisa** | 52 bits vs 23 bits = mayor capacidad de dígitos significativos |
| **Funciones matemáticas** | `pow()` y `sqrt()` retornan `double`, preservando precisión en operaciones intermedias |
| **IEEE 754** | Ambas siguen el estándar, pero `double` tiene mayor rango de exponentes |

### 5.3 Recomendación

Para cálculos científicos o precisos, **siempre utilizar `double`** en lugar de `float`, ya que:

1. La mayoría de máquinas modernas no tiene penalización de rendimiento significativa
2. La pérdida de precisión con `float` puede ser catastrófica en sistemas iterativos
3. Las funciones matemáticas de la biblioteca `<math.h>` están optimizadas para `double`

---

## 6. Referencias

- IEEE Standard 754-2019: IEEE Standard for Floating-Point Arithmetic
- Ecuación Cuadrática: ax² + bx + c = 0 → x = (-b ± √Δ) / 2a, donde Δ = b² - 4ac
- Código fuente: `quadatric1.c`

---

**Informe generado**: Análisis de precisión en operaciones de punto flotante en C
**Herramienta**: OpenCode
