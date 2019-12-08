# Image server and ESP32-CAM client for uploading images

ESP32-CAM client code for uploading images to server via HTTP PUT.

.NET core server(runs on linux docker) to receive uploads and server them to HTTP clients.

Client and server also have communication channel to control upload frequency. This is used to optimise bandwidh usage. When server sees requests for image, it raises upload rate.

Still work in progress but it seems to work.

TODO: Add authetication, better logging and diagnostics.

![Camera prototype](/Media/Prototype_image.png)
