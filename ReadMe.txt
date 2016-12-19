@author Santiago Maraggi

DEMO API: "Microsoft Media Foundation"

Microsoft Media Foundation (MMF) es la API nueva de Microsoft para manipulación de archivos multimedia en distintos formatos y para su reproducción en distintos medios disponibles en la familia de sistemas operativos Microsoft Windows únicamente.

MMF viene integrado con el WindowsSDK, que incorporó funcionalidades de Microsoft Windows antes dispersas en otras APIs, como DXSDK.

MMF reemplaza antiguos ActiveX Controls y funcionalidades de Windows Media Player que se podían llamar desde programas para favorecer la protección de contenido con licencias y derechos de autor, principalmente, a pesar de ofrecer también algunas facilidades para flexibilizar más la manipulación de los distintos streams multimedia.

El presente proyecto implementa un sencillo reproductor de video utilizando los servicios de MMF.

Los servicios ofrecidos por MMF son de bajo nivel, a tal punto que el reproductor manipula los buffers de audio y video por separado, puede acceder a su sincronización y a la configuración de un pipeline de reproducción con hardware específico, que en este caso se deja referenciado al monitor y reproductor de audio por defecto.

Para su desarrollo se consultó la documentación on line de Microsoft y se tomaron partes de código de ejemplo publicadas, siendo necesarias algunas adaptaciones para componer el reproductor completo y responder a algunas necesidades previsibles para aplicaciones que deban manipular y reproducir videos.

Instrucciones de uso
********************

Ejecutar el programa

Al iniciarse el programa se inicializa también el módulo de video (CPlayer).

En el menú seleccionar la opción "Cargar Video". Esto cargará el video por defecto configurado en la aplicación. Para dar comienzo y pausar la reproducción se utiliza la barra espaciadora una vez cargado el video.

Utilizar teclas "q" y "w" para saltear a partes predefinidas del video. Aquí se implementa un "pause"-"seek"-"play".

El programa se debe ejecutar desde Visual Studio 2010 debido a que los dos archivos de video están en el directorio del proyecto. Para ejecutar el programa por fuera del Visual Studio se debe copiar los dos archivos de video al directorio de ejecución del programa.

A continuación se detallan todas las teclas y sus respectivos comandos que se pueden utilizar.

Se puede cerrar la aplicación en cualquier momento, pasar al video alternativo y nuevamente al original con la tecla ENTER y ejecutar en general estos comandos en cualquier orden.

Esta lista de comandos se encuentra también documentada en el archivo "MediaFoundationPlayer.cpp". Los comandos se ejecutan con la tecla directamente en minúscula.
// **************************************************************************
// **************************************************************************
// **************************************************************************
// *********** DEMO VIDEO PLAYER CON MICROSOFT MEDIA FOUNDATION *************

// Ejecutar la aplicación y seleccionar del menú principal "Reproducir Video"

// Uso general:
// En el menú superior elegir la opción "Cargar video"
// Luego iniciar la reproducción con la barra espaciadora
// Utilizar el resto de los controles...
// Cerrar la aplicación al terminar

// Mapa de teclas de control de reproducción

// ESPACIO - Play / Pausa
// Q - Salto hacia atrás 5 segundos
// W - Salto hacia adelante 5 segundos
// E - Volumen de audio máximo 1.0
// R - Volumen de audio 0.5
// T - Volumen de audio nulo 0.0 (mute)
// A - Velocidad de reproducción x1.0 (normal)
// S - Velocidad de reproducción x2.0 (rápido)
// D - Velocidad de reproducción x0.5 (lento)
// Z - Velocidad de reproducción x4.0 (muy rápido)
// X - Deprecated
// ENTER - Cambiar de video

// **************************************************************************
// **************************************************************************
// **************************************************************************

Al salir del programa se cierra el módulo de video (CPLayer). Si el video estaba en reproducción se lo detiene previamente para proceder a cerrar el módulo de video correctamente.

Referencias
***********

https://msdn.microsoft.com/en-us/library/windows/desktop/ms703190(v=vs.85).aspx
https://msdn.microsoft.com/en-us/library/windows/desktop/ms703190(v=vs.85).aspx
https://msdn.microsoft.com/en-us/library/windows/desktop/dd979592(v=vs.85).aspx
https://msdn.microsoft.com/en-us/library/windows/desktop/dd979593(v=vs.85).aspx
https://msdn.microsoft.com/en-us/library/windows/desktop/dd979594(v=vs.85).aspx
https://msdn.microsoft.com/en-us/library/windows/desktop/dd979595(v=vs.85).aspx
https://msdn.microsoft.com/en-us/library/windows/desktop/dd979596(v=vs.85).aspx
https://msdn.microsoft.com/en-us/library/windows/desktop/dd979597(v=vs.85).aspx
https://msdn.microsoft.com/en-us/library/windows/desktop/dd979598(v=vs.85).aspx
https://msdn.microsoft.com/en-us/library/windows/desktop/ff728866(v=vs.85).aspx
https://msdn.microsoft.com/en-us/library/windows/desktop/ms697048(v=vs.85).aspx
https://msdn.microsoft.com/en-us/library/windows/desktop/ee892373(v=vs.85).aspx
https://msdn.microsoft.com/en-us/library/windows/desktop/dd389281(v=vs.85).aspx#getting_file_duration
https://msdn.microsoft.com/en-us/library/windows/desktop/ms703153(v=vs.85).aspx
http://stackoverflow.com/questions/19211610/how-can-i-read-the-info-of-a-media-file-using-visual-c
https://msdn.microsoft.com/en-us/library/windows/desktop/bb762197(v=vs.85).aspx
https://social.msdn.microsoft.com/Forums/windowsdesktop/en-US/41b62ea7-dd6c-4963-b251-280d2f075c71/is-it-possible-to-get-the-number-of-video-frames-for-a-file-using-the-source-reader?forum=mediafoundationdevelopment
