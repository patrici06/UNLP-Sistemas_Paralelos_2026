# Análisis de Rendimiento - Producto de Matrices (ej-2.c)
## Optimizaciones Aplicadas: O3 + CPU Affinity + B Transpuesta

---

## 📊 RESULTADOS COMPLETOS

### **1. Matriz 512×512 (0.25M elementos)**
| Threads | Tiempo (s) | Speedup | Eficiencia |
|---------|-----------|---------|-----------|
| 1 | 0.127423 | 1.00x | 100% |
| 2 | 0.079353 | 1.61x | 80.5% |
| 4 | 0.042289 | 3.01x | 75.3% |
| 8 | 0.033493 | 3.80x | 47.5% |

**Observaciones:**
- Excelente escalabilidad hasta 4 threads
- Con 8 threads hay contención (overhead > beneficio)

---

### **2. Matriz 1024×1024 (1.0M elementos)**
| Threads | Tiempo (s) | Speedup | Eficiencia |
|---------|-----------|---------|-----------|
| 1 | 1.046343 | 1.00x | 100% |
| 2 | 0.576647 | 1.81x | 90.7% |
| 4 | 0.364266 | 2.87x | 71.8% |
| 8 | 0.354448 | 2.95x | 36.9% |

**Observaciones:**
- Mejor escalabilidad con matriz más grande
- 8 threads: hiperthreading/contención comienza

---

### **3. Matriz 2048×2048 (4.1M elementos)** ⭐
| Threads | Tiempo (s) | Speedup | Eficiencia |
|---------|-----------|---------|-----------|
| 1 | 8.826860 | 1.00x | 100% |
| 2 | 5.251127 | 1.68x | 84.0% |
| 4 | 3.248683 | 2.72x | 68.0% |
| 8 | 4.027115 | 2.19x | 27.4% |

**Observaciones:**
- **Mejor ratio trabajo/overhead**
- 2 threads: 84% eficiencia (excelente)
- 4 threads: 68% eficiencia (buena)
- 8 threads: regresa (NUMA/cache effects)

---

### **4. Matriz 4096×4096 (16.7M elementos)** ✅ COMPLETADO
| Threads | Tiempo (s) | Speedup | Eficiencia |
|---------|-----------|---------|-----------|
| 1 | 64.195 | 1.00x | 100% |
| 2 | 45.171 | 1.42x | 71% |
| 4 | 27.335 | 2.35x | 59% |
| 8 | 36.724 | 1.75x | 22% |

**Observaciones Críticas:**
- **Degradación con T=8**: 36.72s vs 27.34s (4T) = +34% más lento
- Contención de memoria L3 + NUMA effects dominan
- CPU affinity no es suficiente a este nivel de presión
- Óptimo global: T=4 (2.35x speedup, 59% eficiencia)

---

## 📈 ANÁLISIS GENERAL

### Tendencias Clave:

#### **Por Tamaño de Matriz:**
```
512×512:   Pequeño  → Overhead domina
1024×1024: Medio    → Escalabilidad moderada
2048×2048: Grande   → Mejor escalabilidad
4096×4096: Muy Gran → A confirmar
```

#### **Óptimo por Config:**
```
512×512:   2-4 threads (3.01x speedup)
1024×1024: 4 threads (2.87x speedup)
2048×2048: 4 threads (2.72x speedup, 68% efic.)
```

---

## 🎯 Conclusiones

### ✅ Mejoras Aplicadas Funcionan:
1. **B Transpuesta**: Mejora cache locality (acceso secuencial)
2. **CPU Affinity**: Reduce migración de threads
3. **Compilación O3**: Optimizaciones del compilador

### ⚠️ Limitaciones Detectadas:
1. **8 threads**: Rendimiento degrada (contención/NUMA)
2. **Overhead fijo**: Creación de threads domina en matrices pequeñas
3. **Cache L3**: Saturación posible en matrices muy grandes

### 💡 Recomendaciones:
```c
// ✅ USAR PARALELISMO PARA:
- Matrices N ≥ 1024
- Threads T ≤ 4
- Cómputo intensivo (O(n³))

// ❌ EVITAR PARA:
- Matrices N < 512
- Threads > num_cores
- Memory bandwidth limited ops
```

---

## 📊 Ratio de Análisis

| Métrica | Valor | Interpretación |
|---------|-------|----------------|
| Speedup máx (2048²) | 2.72x | Bueno para 4 cores |
| Eficiencia máx | 84% | Excelente (2threads 2048²) |
| Break-even point | ~512² | Overhead se amortiza aquí |
| Punto óptimo | ~2048² + 4T | Mejor relación costo-beneficio |

---

**Generado:** 17 Mayo 2026  
**Compilación:** gcc -O3 -lpthread -std=c99
