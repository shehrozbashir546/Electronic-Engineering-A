// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

bool show_map = true;

int initial_speed = 1000;

//0. Index(serial number), 1. Arrival time 2. From(NESW/0-3), 3. To/Direction(straight 0, right 1, left 2), 4. Position x, 5. Position y, 6. Speed
int cars[1][7] = {{0, 0, 0, 0, 0, 0, 0}};

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
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void Car(void *parameter) {
  //0. Index(serial number), 1. Arrival time 2. From(NESW/0-3), 3. Direction(straight 0, right 1, left 2), 4. Position x, 5. Position y, 6. Speed
  int car[7] = {*((int*)parameter), 0, 0, 0, 0, 0, 0};
  int checkpoint[2][3] = {{0, 0, 0}, {0, 0, 0}}; //x and y coordinates of checkpoints
  int destination = 0;
  int old_pos[2] = {0, 0};

  //random car atrributes
  car[1] = millis(); //time car creation = car arrival time
  car[3] = random(3);
  car[6] = initial_speed;
  int command = 1;

  //find x and y position and checkpoint 1
  do {
    car[2] = random(4);
    switch (car[2]) {
      case 0: //N
        car[4] = 7; //x
        car[5] = 15; //y
        checkpoint[0][0] = 7;
        checkpoint[1][0] = 9;
        break;
      case 1: //E
        car[4] = 15; //x
        car[5] = 8; //y
        checkpoint[0][0] = 9;
        checkpoint[1][0] = 8;
        break;
      case 2: //S
        car[4] = 8; //x
        car[5] = 0; //y
        checkpoint[0][0] = 8;
        checkpoint[1][0] = 6;
        break;
      case 3: //W
        car[4] = 0; //x
        car[5] = 7; //y
        checkpoint[0][0] = 6;
        checkpoint[1][0] = 7;
        break;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  } while (world[car[4]][car[5]] == 'X'); //saerch for new starting point that is free
  world[car[4]][car[5]] = 'X';
  //find destination
  switch (car[3]) {
    case 0: //staright
      destination = car[2] + 2;
      if (destination > 3) {
        destination -= 4;
      }
      break;
    case 1: //right
      destination = car[2] - 1;
      if (destination < 0) {
        destination = 3;
      }
      break;
    case 2: //left
      destination = car[2] + 1;
      if (destination > 3) {
        destination = 0;
      }
      break;
  }

  //Find checkpoint 2 and 3 with the help of the destination
  switch (destination) {
    case 0: //N
      checkpoint[0][1] = 8;
      checkpoint[1][1] = 9;
      checkpoint[0][2] = 8;
      checkpoint[1][2] = 15;
      break;
    case 1: //E
      checkpoint[0][1] = 9;
      checkpoint[1][1] = 7;
      checkpoint[0][2] = 15;
      checkpoint[1][2] = 7;
      break;
    case 2: //S
      checkpoint[0][1] = 7;
      checkpoint[1][1] = 6;
      checkpoint[0][2] = 7;
      checkpoint[1][2] = 0;
      break;
    case 3: //W
      checkpoint[0][1] = 6;
      checkpoint[1][1] = 8;
      checkpoint[0][2] = 0;
      checkpoint[1][2] = 8;
      break;
  }

  if (!show_map)
  {
    Serial.print("Car ");
    Serial.print(car[0]);
    Serial.println(" is announcing itself.");

    Serial.print("Car attributes: ");
    for (int i = 0; i < 7; i++)
    {
      Serial.print(car[i]);
      Serial.print(" ");
    }
    Serial.println();
  }

  //driving
  bool arrived = false, drive = true;
  int target = 0; //used for reaching checkpoints
  while (!arrived) {
    old_pos[0] = car[4];
    old_pos[1] = car[5];

    vTaskDelay(car[6] / portTICK_PERIOD_MS);

    switch (command) {
      case 0: //go
        drive = true;
        break;
      case 1: //slow down (stops at checkpoint 1 or behind another car)

        break;
      case 2: //stop (wait)
        drive = false;
        break;
    }
    if (drive)
    {
      switch (target) {
        case 0: //to checkpoint 1 -> drive straight forward
          switch (car[2]) {
            case 0: //N
              car[5] -= 1; //y
              break;
            case 1: //E
              car[4] -= 1; //x
              break;
            case 2: //S
              car[5] += 1; //y
              break;
            case 3: //W
              car[4] += 1; //x
              break;
          }


          if (world[car[4]][car[5]] == 'X') //Stop to prevent crash -> get in line!
          {
            car[4] = old_pos[0];
            car[5] = old_pos[1];
          }

          if (car[4] == checkpoint[0][0] && car[5] == checkpoint[1][0])
          {
            if (command == 0)
            {
              target = 1;
            }
            else
            {
              command = 2;
            }
          }
          break;
        case 1: //to checkpoint 2 -> drive straight forward, then change direction
          switch (car[2]) {
            case 0: //N
              car[5] -= 1; //y
              if (car[5] == checkpoint[1][1]) //Has the car reached the checkpoint on one axis? Change driving direction!
              {
                car[2] = destination + 2; //Trick 17 -> My code drives away from its origin. With this I change the origin to the opposite of the destination. This means, it drives to the destination!
                if (car[2] > 3) {
                  car[2] -= 4;
                }
              }
              break;
            case 1: //E
              car[4] -= 1; //x
              if (car[4] == checkpoint[0][1])
              {
                car[2] = destination + 2;
                if (car[2] > 3) {
                  car[2] -= 4;
                }
              }
              break;
            case 2: //S
              car[5] += 1; //y
              if (car[5] == checkpoint[1][1])
              {
                car[2] = destination + 2;
                if (car[2] > 3) {
                  car[2] -= 4;
                }
              }
              break;
            case 3: //W
              car[4] += 1; //x
              if (car[4] == checkpoint[0][1])
              {
                car[2] = destination + 2;
                if (car[2] > 3) {
                  car[2] -= 4;
                }
              }
              break;
          }
          if (car[4] == checkpoint[0][1] && car[5] == checkpoint[1][1])
          {
            target = 2;
          }
          break;
        case 2:
          switch (car[2]) {
            case 0: //N
              car[5] -= 1; //y
              break;
            case 1: //E
              car[4] -= 1; //x
              break;
            case 2: //S
              car[5] += 1; //y
              break;
            case 3: //W
              car[4] += 1; //x
              break;
          }
          if (car[4] == checkpoint[0][2] && car[5] == checkpoint[1][2])
          {
            arrived = true;
          }
          break;
      }
      world[old_pos[0]][old_pos[1]] = ' ';
      world[car[4]][car[5]] = 'X';

      if (!show_map) {
        Serial.print(car[0]);
        Serial.print(" ");
        Serial.print(car[4]);
        Serial.print(" ");
        Serial.print(car[5]);
        Serial.print(" ");
        Serial.print(old_pos[0]);
        Serial.print(" ");
        Serial.print(old_pos[1]);
        Serial.print(" ");
        Serial.println(destination);
      }
    }
  }
  if (!show_map) {
    world[car[4]][car[5]] = ' ';
    Serial.print(car[0]);
    Serial.println(" arrived");
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

  vTaskDelay(2000 / portTICK_PERIOD_MS);

  for (int i = 4; i < 8; i++) {
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
