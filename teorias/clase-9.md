Esta clase tiene la intencionalidad de brindar un panorama sobre la tendencias del mercado sobre este campo especifico de la infromatica, asumido las tendencias que generalmente se vinculan al uso de menor costo energeticos para mejores bechmarks y un adjunto sobre las diferentes supercomputadoras, continuaremos sobre uno de los componentes que ha permitido en buena parte los clusters y de amplio interes para estas cuestiones: 


### GPUs 
Disenadas originalmente para procesamiento de graficos debido a su gran potencia de calculo, loas empresas fabricantes comenzaron a aumentar su grado de programacion. 
Motivados por el surgimiento de nuevas tecnicas, lenguajes y herrramientas para la programacion de GPU, lo cual permite utilizar a las mismas como arquitecturas paralelas para resolverr problemas de proposito general. 

La significativa difrencia de rendimiento entre las CPUs y las GPUs se debe a que sus filosofias de diseno son muy distintas. 
	- Las Cpus destinan los recursos de silicio principalmente a memorias cache y a nucleos de compleja organizacion que permitan explotar ILP. 
	- GPUs emplean la mayor parte del silicio en unidades funcionales. Cada una de ellas tiene un conjunto de nucleos simples que comparten logica de control, ejecutan instrucciones een orden y operan en grupos como si fuera un procesador vectorial.

### GPUs 
	A diferencia de las CPUs, las GPUs tienen una jerarquia de memoria compleja: 
	- Memoria global: es una memoria off-chip que sirve de memoria principal. Ancho de banda limitado y latencia alta comparado a las memorias on-chi o cache. 
	- Memoria compartida: Memoria on-chip caracterizada por alto ancho de banda y baja latencia. Se administra porr software y es accesible por todos los hilos activos de un multiprocesador. 	
	- Memoria de constantes: memoria raida pero pequena y de solo lectura, ubicada dentro de la memoria global. Es visibe por todos los hilos. 
	- Memoria de texturas: Similar a la de constantes. Memoria off-chip optimizada para localidad especial 2D. 
	
Las primeras aplciaciones no graficas eran programadas termino de operaciones graficas usando lenguajes como OpenGL o DirectX -> Reultaba engorroso y propenso a errores. 

Tanto la industria como la academia propusieron varios lenguajes que permiten abstrarse de los graficos:
	- CUDA,OpenCL, OpenACC, y SYCL. 
Al dia de hoy son 3 empresas las que comparten el mercado de las GPUs. 
	- Intel es la mas grande, pero solo domina el segmento correspondiente a placas integradas y de bajo rendimiento. 
	- En el segmento de alto rendimiento AMD Y NVIDIA, aunque NVIDIA supera ampliamente a AMD
	
### GPUs
	Las Gpu son arquitecturas de memoria compartidas inspiradas en SIMD 
	Por sus caracteristicas se adaptan mejor a aplicaciones que admiten paralelismo de datos, especialmente aquellas queson intensivas en computo  (CPU-bound). 

### CUDA:
  - Estandar de facto para programacion de GPUs en HPC
  - Modelo de ejecucion (host-device) y arquitectura de memoria de CUDA
  - El host es el responsable de administrar la memoria del dispositivos y sus transferencias, ademas de invocar la ejecucion de los kernels. 
  - Un kernel es un trozo de codigo que ejecutan miles de hilos primitivos en paralelo en la GPU. 
### OpenCL:
  - Estandar para programacion paralela multi-plataforma
  - Modelo de ejecucion (host-device) y arquitectura de memoria de OpenCL 
  Similar a Cuda pero con un efoque mas general ya que aplica a GPUs. 
### SYCL: 
 - Estandar para programacion paralela multi-plataforma
 - Basado en OpenCL pero buscando reducir esfuerzo de programacion. 
 	- Memoria compartida unificada	
	- Reducciones paralelas (integradas)
	- Funciones a nivel de work-groups y sub-groups
	- Accessors
	- Interoperabilidad con otras APIs
### GPUs de NVIDIA
	2014 -> pascal
	- rediseno dde los SMXs pasando a llamarse SMMs -> Hasta 1.35x de mejora en rendimiento por core y 2x de mejora en eficiencia energetica. 
	- Integracion con CPUs de ARM
	- Esquema de memoria unificada entre CPU y GPU para evitar reservas de memoria individuales (por software). 
	- Soporte para precision mixta -> metodo que ultiliza diferentes niveles de precision dentro de una sola operacion para lograr eficiencia computacional sin afectar el resultado final. 
	2017 -> Volta
	- Orientada al uso de machine learning -> incorpora soporte para precision media (float de 16 bits) mediante nucleos especificos (Tensor cores)
	- Adopta HBM2
	- NVLINK 2.0
	2018 -> Turing
	- Similar a Volta, turing esta orientada al sector consumidor
	- Incorpora soporte especifico para Ray-Tracing -> nucleos dedicados. 
### GPUs de AMD: 
	En 2006 compran la empresa ATI que permitio incrementar su capacidad de producir e innovar hadware grafico. 	
	Con la adquisicion de ATI AMD comenzo a innovar el hardware grafico -> Un resultado son las unidades de procesamiento acelerado (APU), luego renombrados comenrcialmente a AMD Fusion 
	Las APUs combinan una CPu y una GPU en el mismo chip, y se basan en la arquitectura GCN -> Son la base de los AMD Ryzen actuales. 
	Son una buena opcion desde la perspectiva precio-rendimiento, (buenos cocientes de eficiencia energetica). 
	
### Memoria de Alto Ancho de Banda HBM: 
Una de las innovaciones mass importantes de AMD en los ultimos anos es su tecnologia Memoria de Alto Ancho de Banda. 
- HBM es un nuevo tipo de memoria RAM que organiza los chips de memoria en forma vertica y apilada (memoria 3D) y que puede ser aprovechado tanto por GPUs como CPUs. 
- Mejora el ahorro de espacio y considerablemente en la velocidad de comunicacion asi como la eficiencia energetica. 
- Envidia adopta este diseno apartir de 2016. 

### GPUS y Procesadores Hibridos de Intel: 
Intel Xe -> la familia de GPUs Xe consiste de un conjunto de micro arquitecturas: 
	 - Xe-LP -> Integradas y de bajo consumo 
	 - Xe-HPG -> Gaming (alto rendimiento)
         - Xe-HP -> Datacenter(alto rendimiento)
	 - Xe-HPC -> HPC

### FPGAs
UN FPGA consiste de circuitos integrados reconfigurables compuestos por interconexiones programables que unene bloques logicos programables, bloques de memoria embebidos y bloques DSPs -> hardware programable. 
La comunicacion con el exterior se realiza a traves de los bloques de E/S, los cuales se organizan en forma de anillo alrededor de la circunferencia del dispositivo. 

Las CPUS y GPUs presentan topologias y rutas de datos estaticas para procesar las instrucciones de los programas -> los recursos de las FPGAs puede ser configurados e interconectadso para crear pipelines de instrucciones a medida en los cuales procesar los datos

Por ejemplo ssi un algoritmo requiere cierto tipo de aritmetrica con entreos, vale la pena destinar recursos para punto flotante?, para otro tipo de opraciones no sean las de interes? 

Es un cambio de paradigma, siempre opto por que la aplicacion se programe para adaptarse al hardware, FPGAs se desarrolla el hardware para que se adapte a la aplicacion. Un cambio de paradigma completo. 

Si bien tanto la frecuencia del reloj como el pico de rendimiento suelen ser mas bajos que los correspondientes a las CPUs y a las GPUs, la capacidad de configurar el hardware ara que se adapte a problema especifico a resolver le da la posibilida de obtener mejores rendimientos. 
Asi ademas no hay desperdicio de recursos de sillicio, en general son mas eficientes desde el punto de vista energetico -> aqui su mayor atractivo. 

Es importante saber que no siempre se conveciente uso de FPGAs. Entre otras caracteristicas para obtener alto rendimiento se requiere: 
	- Operaciones aritmetricas simples (punto fijo -> mejor)
	- Amplio paralelismo de datos 
	- Estructuras de control regulares y sencillas
FPGAs tiene un complejo flujo de programacion

Opcioens para Entrada de diseno: CAD vs HDL vs HLS
 - Los disenos generados a a partir de herramientas CAD son mas faciles de leer y comprender, aunque solo suelen funcionar en proyectos pequenos.
- Para disenos complejos, la opcion tradicional es el uso de lenguajes de descripcion de hardware HDL, un enfoque basado en codigo. 
- En los ultimos anos, se han desarrollado alternaticas de alto nivel para los HDLs, HLL

HDL -> lenguaje de programacion especializado que se utiliza para definir la estructura de diseno y operacion de circuitos electronicos, especialemente los digitales. 
algunas opciones son Verilog y VHDL.
