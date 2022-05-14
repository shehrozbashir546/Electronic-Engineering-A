// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

int cars[4] = {3, 1, 0, 2};
bool gos[4] = {false, false, false, false}; //sending command from system to car
bool left[4] = {false, false, false, false}; //sending command from car to system

//void Car(int car_index) {
void Car(void *parameter) {
  int car_index = *((int*)parameter);
  //while(car_index == 0){}; //Wait until index is assigned

  Serial.print("Car ");
  Serial.print(car_index);
  Serial.println(" is created.");

  while (!gos[car_index]) {
    //Wait till go
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }

  Serial.print("Car ");
  Serial.print(car_index);
  Serial.println(" is leaving.");
  left[car_index] = true;

  while (1) {};//"idle"
}

void Priority_Sort_Duck(void *parameter) {
  int waiting_line[4] = {0, 0, 0, 0};
  int cars_copy[4];

  for (int i = 0; i < 4; i++) {
    cars_copy[i] = cars[i];
  }

  int highest = 0;
  bool first = true;
  Serial.print("Order of passing the corsssection: ");
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (cars_copy[j] != 999) {
        if (!first && (cars_copy[j] > cars_copy[highest])) {
          highest = j;
        }
        else if (first) { //if we run the loop the first time we assume a highest
          highest = j;
          first = false;
        }
      }
    }
    first = true;
    waiting_line[i] = highest;
    cars_copy[highest] = 999;
    Serial.print(highest);
    Serial.print(" ");
  }
  Serial.println();

  int car_in_line = 0;
  first = true;
  do{
    if(first){
    Serial.print("System: \"Go Car ");
    Serial.print(waiting_line[car_in_line]);
    Serial.println("\"");
    gos[waiting_line[car_in_line]]=true;
    first = false;
    }
    if(left[waiting_line[car_in_line]]==true){
      car_in_line++;
      first=true;
    }
   }while(car_in_line < 4);

  Serial.println("System is on idle.");
  while (1) {};//"idle"
}

void setup() {
  Serial.begin(115200);

  //Create 4 cars and give them their index
  for (int i = 0; i < 4; i++) {
    xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
      Car,  // Function to be called
      "Car" + i,   // Name of task
      2048,         // Stack size (bytes in ESP32, words in FreeRTOS)
      (void*)&i,         // Parameter to pass to function
      1,            // Task priority (0 to configMAX_PRIORITIES - 1)
      NULL,         // Task handle
      app_cpu);     // Run on one core for demo purposes (ESP32 only)
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }

  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
    Priority_Sort_Duck,  // Function to be called
    "Priority_Sort_Duck",   // Name of task
    4096,         // Stack size (bytes in ESP32, words in FreeRTOS)
    NULL,         // Parameter to pass to function
    1,            // Task priority (0 to configMAX_PRIORITIES - 1)
    NULL,         // Task handle
    app_cpu);     // Run on one core for demo purposes (ESP32 only)
}

void loop() {
  // put your main code here, to run repeatedly:

}
