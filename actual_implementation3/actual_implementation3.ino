//defining priorities
#define straight 20
#define right 30
#define left 10
#define waitingp 10
#define speedp 10
#define carp 10

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

bool show_map = true;

int initial_speed = 1200;

const int car_size = 10;
//0. Index(serial number), 1. Arrival time 2. From(NESW/0-3), 3. To/Direction(straight 0, right 1, left 2), 4. Position x, 5. Position y, 6. Speed, 7. command, 8. priority coins
int cars[car_size][9];
bool finished[car_size];
static SemaphoreHandle_t car_mutex;

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

void TMS(void *parameter) {
  int queue[car_size];
  byte turn[4];
  int origin[4] = {0, 0, 0, 0}; //NESW
  int cars_copy[car_size];
  int tms[4][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}; //For every NESW origin a direction exists (straight, right...). 0 = Free choice, 1 = Go, >1 = Stop!
  bool origin_bonus[car_size];
  bool scheduled[car_size];
  unsigned long waiting_old = 0;

  for (int i = 0; i < car_size; i++) //initializing
  {
    scheduled[i] = false;
    queue[i] = 0;
    origin_bonus[i] = false;
    finished[i] = false;
    xSemaphoreTake(car_mutex, portMAX_DELAY); //go sure it is written only once at a time
    for (int j = 0; j < 9; j++)
    {
      cars[i][j] = 0;
    }
    cars[i][0] = -1;
    xSemaphoreGive(car_mutex);
  }

  while (1) //Entering the scheduling loop
  {
    vTaskDelay(100 / portTICK_PERIOD_MS);
    unsigned long waiting = millis();
    for (int i = 0; i < car_size; i++)
    {
      if (cars[i][0] != -1 && cars[i][8] == 0) //Arrival time = 0? We don't care about those. We only give the following points to new arriving cars.
      {
        //Give Priority points based on direction
        switch (cars[i][3]) {
          case 0:
            cars[i][8] += straight;
            break;
          case 1:
            cars[i][8] += right;
            break;
          case 2:
            cars[i][8] += left;
            break;
        }

        if (cars[i][6] != 0) {
          cars[i][8] += 1000 / cars[i][6] * speedp; //speedp * steps per second
        }

        origin[cars[i][2]] += 10;
        origin_bonus[i] = true;
      }
    }
    for (int i = 0; i < car_size; i++)
    {
      if (cars[i][0] != -1 && origin_bonus[i])
      {
        cars[i][8] += origin[cars[i][2]];
        origin_bonus[i] = false;
      }
    }

    if (waiting - waiting_old >= 1000) { //Every second free priority coins! Everyone likes free stuff! ^^
      for (int i = 0; i < car_size; i++) {
        if (cars[i][0] != -1)
        {
          waiting_old = waiting;
          cars[i][8] += waitingp;
        }
      }

      /*for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
          Serial.print(" ");
          Serial.print(tms[i][j]);
        }
        Serial.println();
      }
      for (int i = 0; i < 10; i++) {
        Serial.print(queue[i]);
        Serial.print(" ");
        Serial.print(cars[queue[i]][8]);
        Serial.print("; ");
      }
      Serial.println();*/
    }

    for (int i = 0; i < car_size; i++) {
      cars_copy[i] = cars[i][8];
    }

    int highest = 0;
    bool first = true;
    for (int i = 0; i < car_size; i++) {
      for (int j = 0; j < car_size; j++) {
        if (cars_copy[j] != -1) {
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
      queue[i] = highest;
      cars_copy[highest] = -1;
    }

    for (int i = 0; i < car_size; i++) { //Block every illegal crossing
      if (!scheduled[queue[i]] && (tms[cars[queue[i]][2]][cars[queue[i]][3]] == 1 || tms[cars[queue[i]][2]][cars[queue[i]][3]] == 0) && cars[queue[i]][0] != -1 && !finished[queue[i]])
      {
        scheduled[queue[i]] = true;

        turn[0] = cars[queue[i]][2];
        for (int j = 1; j < 4; j++)
        {
          turn[j] = turn[j - 1] + 1;
          if (turn[j] > 3) {
            turn[j] -= 4;
          }
        }

        tms[turn[0]][cars[queue[i]][3]] = 1;

        switch (cars[queue[i]][3]) {
          case 0:
            tms[turn[1]][0] += 2;
            tms[turn[1]][1] += 0;
            tms[turn[1]][2] += 2;

            tms[turn[2]][0] += 0;
            tms[turn[2]][1] += 0;
            tms[turn[2]][2] += 2;

            tms[turn[3]][0] += 2;
            tms[turn[3]][1] += 2;
            tms[turn[3]][2] += 2;
            break;
          case 1:
            tms[turn[1]][0] += 2;
            tms[turn[1]][1] += 0;
            tms[turn[1]][2] += 2;

            tms[turn[2]][0] += 0;
            tms[turn[2]][1] += 0;
            tms[turn[2]][2] += 2;

            tms[turn[3]][0] += 0;
            tms[turn[3]][1] += 0;
            tms[turn[3]][2] += 0;
            break;
          case 2:
            tms[turn[1]][0] += 2;
            tms[turn[1]][1] += 0;
            tms[turn[1]][2] += 2;

            tms[turn[2]][0] += 2;
            tms[turn[2]][1] += 2;
            tms[turn[2]][2] += 2;

            tms[turn[3]][0] += 2;
            tms[turn[3]][1] += 2;
            tms[turn[3]][2] += 2;
            break;
        }
        xSemaphoreTake(car_mutex, portMAX_DELAY); //go sure it is written only once at a time
        cars[queue[i]][7] = 0; //Go car!
        xSemaphoreGive(car_mutex);
      }
    }

    for (int i = 0; i < car_size; i++) { //reset blockades if it left the cross secion
      if (scheduled[i] && finished[i]) {
        scheduled[i] = false;

        turn[0] = cars[i][2];
        for (int j = 1; j < 4; j++)
        {
          turn[j] = turn[j - 1] + 1;
          if (turn[j] > 3) {
            turn[j] -= 4;
          }
        }

        tms[turn[0]][cars[i][3]] = 0;

        switch (cars[i][3]) {
          case 0:
            tms[turn[1]][0] -= 2;
            tms[turn[1]][1] -= 0;
            tms[turn[1]][2] -= 2;

            tms[turn[2]][0] -= 0;
            tms[turn[2]][1] -= 0;
            tms[turn[2]][2] -= 2;

            tms[turn[3]][0] -= 2;
            tms[turn[3]][1] -= 2;
            tms[turn[3]][2] -= 2;
            break;
          case 1:
            tms[turn[1]][0] -= 2;
            tms[turn[1]][1] -= 0;
            tms[turn[1]][2] -= 2;

            tms[turn[2]][0] -= 0;
            tms[turn[2]][1] -= 0;
            tms[turn[2]][2] -= 2;

            tms[turn[3]][0] -= 0;
            tms[turn[3]][1] -= 0;
            tms[turn[3]][2] -= 0;
            break;
          case 2:
            tms[turn[1]][0] -= 2;
            tms[turn[1]][1] -= 0;
            tms[turn[1]][2] -= 2;

            tms[turn[2]][0] -= 2;
            tms[turn[2]][1] -= 2;
            tms[turn[2]][2] -= 2;

            tms[turn[3]][0] -= 2;
            tms[turn[3]][1] -= 2;
            tms[turn[3]][2] -= 2;
            break;
        }

      }
    }

  }
}

void Car(void *parameter) {
  //0. Index(serial number), 1. Arrival time 2. From(NESW/0-3), 3. Direction(straight 0, right 1, left 2), 4. Position x, 5. Position y, 6. Speed, 7. command
  int car[8] = {*((int*)parameter), 0, 0, 0, 0, 0, 0};
  int checkpoint[2][3] = {{0, 0, 0}, {0, 0, 0}}; //x and y coordinates of checkpoints
  int destination = 0;
  int old_pos[2] = {0, 0};
  char output = 'X';

  //random car atrributes
  car[1] = millis(); //time car creation = car arrival time
  car[3] = random(3);
  car[6] = initial_speed;
  car[7] = 1; //Stop at checkpoint 1 -> more secure

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
    vTaskDelay(100 / portTICK_PERIOD_MS); //some delay
  } while (world[car[4]][car[5]] != ' '); //saerch again if current spot is blocked

  //find destination
  switch (car[3]) {
    case 0: //staright
      destination = car[2] + 2;
      if (destination > 3) {
        destination -= 4;
      }
      output = 'S';
      break;
    case 1: //right
      destination = car[2] - 1;
      if (destination < 0) {
        destination = 3;
      }
      output = 'R';
      break;
    case 2: //left
      destination = car[2] + 1;
      if (destination > 3) {
        destination = 0;
      }
      output = 'L';
      break;
  }
  world[car[4]][car[5]] = output; //place car

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

  if (!show_map) //print some logs if the map is turned off
  {
    Serial.print("Car ");
    Serial.print(car[0]);
    Serial.println(" is announcing itself.");

    Serial.print("Car attributes: ");
    for (int i = 0; i < 8; i++)
    {
      Serial.print(car[i]);
      Serial.print(" ");
    }
    Serial.println();
  }

  //communication: add car to the global car array
  xSemaphoreTake(car_mutex, portMAX_DELAY); //go sure it is written only once at a time
  for (int i = 0; i < 8; i++)
  {
    cars[car[0]][i] = car[i];
  }
  xSemaphoreGive(car_mutex);

  //driving
  bool arrived = false, drive = true;
  int target = 0; //used for reaching checkpoints
  while (!arrived) {
    old_pos[0] = car[4];
    old_pos[1] = car[5];

    vTaskDelay(car[6] / portTICK_PERIOD_MS);
    car[7] = cars[car[0]][7];
    switch (car[7]) {
      case 0: //go
        drive = true;
        break;
      case 1: //slow down (stops at checkpoint 1 or behind another car)

        break;
      case 2: //stop (wait)
        drive = false;
        xSemaphoreTake(car_mutex, portMAX_DELAY); //tell tms that the car stopped
        cars[car[0]][7] = 2;
        xSemaphoreGive(car_mutex);
        break;
      default:
        break;
    }
    if (drive)
    {
      switch (target) {
        case 0: //to checkpoint 1 -> drive straight forward
          if (car[4] != checkpoint[0][0] || car[5] != checkpoint[1][0])
          {
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

            if (world[car[4]][car[5]] != ' ') //Stop to prevent crash -> get in line!
            {
              car[4] = old_pos[0];
              car[5] = old_pos[1];
            }
          }

          if (car[4] == checkpoint[0][0] && car[5] == checkpoint[1][0])
          {
            if (car[7] == 0)
            {
              target = 1;
            }
            else
            {
              xSemaphoreTake(car_mutex, portMAX_DELAY); //tell tms that the car stopped
              cars[car[0]][7] = 2;
              xSemaphoreGive(car_mutex);
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
            xSemaphoreTake(car_mutex, portMAX_DELAY);
            cars[car[0]][1] = 0;
            finished[car[0]] = true;
            xSemaphoreGive(car_mutex);
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

      world[car[4]][car[5]] = output;


    }
    if (!show_map) {
      Serial.print(car[0]);
      Serial.print(" ");
      Serial.print(car[4]);
      Serial.print(" ");
      Serial.print(car[5]);
      Serial.print(" ");
      Serial.print(car[7]);
      Serial.print(" ");
      Serial.println(destination);
    }
  }
  world[car[4]][car[5]] = ' ';
  xSemaphoreTake(car_mutex, portMAX_DELAY); //tell tms that the car stopped
  cars[car[0]][7] = 3;
  cars[car[0]][0] = -1;
  xSemaphoreGive(car_mutex);
  if (!show_map) {
    Serial.print(car[0]);
    Serial.println(" arrived");
  }
  while (1) {};//"idle"
}

void setup() {
  Serial.begin(115200);
  car_mutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
    TMS,  // Function to be called
    "TMS",   // Name of task
    16384,         // Stack size (bytes in ESP32, words in FreeRTOS)
    NULL,         // Parameter to pass to function
    1,            // Task priority (0 to configMAX_PRIORITIES - 1)
    NULL,         // Task handle
    app_cpu);     // Run on one core for demo purposes (ESP32 only)
  vTaskDelay(1 / portTICK_PERIOD_MS);

  delay(10);

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
