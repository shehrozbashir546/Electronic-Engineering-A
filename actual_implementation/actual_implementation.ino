// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

bool show_map = true;

int initial_speed = 2;

//0. Index(serial number), 1. Arrival time 2. From(NESW/0-3), 3. To/Direction(straight 0, right 1, left 2), 4. Position(16x16), 5. Speed
int cars[1][6] = {{0, 0, 0, 0, 0, 0}};

//Index and assignned priority
int prioritys[1][2] = {{0, 0}};

//Map
char world[16][16] = {
  {' ', ' ', ' ', ' ', ' ', ' ', 'I', ' ', ' ', 'I', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', 'I', ' ', ' ', 'I', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', 'I', ' ', ' ', 'I', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', 'I', ' ', ' ', 'I', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', 'I', ' ', ' ', 'I', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', 'I', ' ', ' ', 'I', ' ', ' ', ' ', ' ', ' ', ' '},
  {'-', '-', '-', '-', '-', '-', '+', ' ', ' ', '+', '-', '-', '-', '-', '-', '-'},
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
  {'-', '-', '-', '-', '-', '-', '+', ' ', ' ', '+', '-', '-', '-', '-', '-', '-'},
  {' ', ' ', ' ', ' ', ' ', ' ', 'I', ' ', ' ', 'I', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', 'I', ' ', ' ', 'I', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', 'I', ' ', ' ', 'I', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', 'I', ' ', ' ', 'I', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', 'I', ' ', ' ', 'I', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', 'I', ' ', ' ', 'I', ' ', ' ', ' ', ' ', ' ', ' '},
};

void Print_Map(void *parameter) {
  while (1)
  {
    if (show_map)
    {
      Serial.println();
      for (int i = 0; i < 16; i++)
      {
        for (int j = 0; j < 16; j++)
        {
          Serial.print(world[i][j]);
        }
        Serial.println();
      }
      Serial.println();
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void Car(void *parameter) {
  int car[6] = {*((int*)parameter), 0, 0, 0, 0, 0};

  //random car atrributes
  car[2] = millis(); //time car creation = car arrival time
  car[3] = random(4);
  car[4] = random(3);
  car[5] = initial_speed;

  if (!show_map)
  {
    Serial.print("Car ");
    Serial.print(car[0]);
    Serial.println(" is announcing itself.");

    Serial.print("Car attributes: ");
    for (int i = 0; i < 6; i++)
    {
      Serial.print(car[i]);
      Serial.print(" ");
    }
    Serial.println();
  }
  while (1) {};//"idle"
}

void setup() {
  Serial.begin(115200);

  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
    Print_Map,  // Function to be called
    "Print_Map",   // Name of task
    2048,         // Stack size (bytes in ESP32, words in FreeRTOS)
    NULL,         // Parameter to pass to function
    1,            // Task priority (0 to configMAX_PRIORITIES - 1)
    NULL,         // Task handle
    app_cpu);     // Run on one core for demo purposes (ESP32 only)
  vTaskDelay(1 / portTICK_PERIOD_MS);

  for (int i = 0; i < 4; i++) {
    xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
      Car,  // Function to be called
      "Car" + i,   // Name of task
      4096,         // Stack size (bytes in ESP32, words in FreeRTOS)
      (void*)&i,         // Parameter to pass to function
      1,            // Task priority (0 to configMAX_PRIORITIES - 1)
      NULL,         // Task handle
      app_cpu);     // Run on one core for demo purposes (ESP32 only)
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void loop() {
}
