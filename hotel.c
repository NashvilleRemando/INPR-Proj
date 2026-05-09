#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ROOMS 100
#define MAX_BOOKINGS 500
#define MAX_AMENITIES 20

// ─── Structs ───────────────────────────────────────────────────────────────────

typedef struct {
  char referenceNumber[10];
  char guestName[50];
  char checkIn[20];
  char checkOut[20];
  int numberOfGuests;
  int numberOfDays;
  char roomType[30];
  int roomNumber;
  float pricePerNight;
  float roomRate;
  float amenitiesTotal;
  float finalAmount;
  float amountReceived;
  float change;
  char paymentMethod[20];
  int isPaid;
} Reservation;

typedef struct {
  int roomNumber;
  char category[30];
  int bedrooms;
  float pricePerNight;
  int isAvailable;
  char guestName[50];
  int numberOfDays;
  char checkIn[20];
  char checkOut[20];
} Room;

typedef struct {
  char code[5];
  char name[60];
  float price;
  char type[20]; // PerNight or PerGuest
} Amenity;

// ─── Forward Declarations ─────────────────────────────────────────────────────

void displayMenu();
void chooseOption();
void listVacant();
void reserve();
void payment(const char *referenceNumber, int fromReserve);
void registry();
void viewDetails();

void generateReferenceNumber(char *output);
int readRoom(FILE *fp, Room *room);
int readAmenities(const char *filename, Amenity *list, int maxCount);
int findBooking(const char *referenceNumber, Reservation *reservation);
void updateBookingPayment(Reservation *reservation);
void printWithCommas(float amount);

// ─── Main ──────────────────────────────────────────────────────────────────────

int main() {
  int choice;
  do {
    displayMenu();

    printf("Selection: ");
    if (scanf("%d", &choice) == 0) {
      while (getchar() != '\n')
        ;
      printf("Invalid input. Please enter a number.\n");
      continue;
    }
    while (getchar() != '\n');

    switch (choice) {
    case 1:
      listVacant();
      break;
    case 2:
      reserve();
      break;
    case 3:
      {
        char refInput[10];
        printf("Enter Reference No: ");
        scanf("%9s", refInput);
        while (getchar() != '\n')
          ;
        payment(refInput, 0);
        break;
      }
    case 4:
      registry();
      break;
    case 5:
      viewDetails();
      break;
    case 6:
      printf("Checkout: coming soon.\n");
      break;
    case 7:
      printf("Exiting ESPLENIN HOTEL system...\n");
      break;
    default:
      printf("Invalid choice. Try again.\n");
    }
  } while (choice != 7);

  return 0;
}

// ─── Menu ──────────────────────────────────────────────────────────────────────

void displayMenu() {
  printf("\n====== ESPLENIN HOTEL ======\n");
  printf("1. Vacancies\n");
  printf("2. Reserve\n");
  printf("3. Payment\n");
  printf("4. Registry\n");
  printf("5. Details\n");
  printf("6. Checkout\n");
  printf("7. Exit\n");
  printf("============================\n");
}

// ─── Option 1: List Vacant Rooms ──────────────────────────────────────────────

void listVacant() {
  FILE *file = fopen("rooms.txt", "r");
  if (!file) {
    printf("Could not open rooms.txt\n");
    return;
  }

  Room room;
  int foundAny = 0;

  printf("\n--- VACANT ROOMS ---\n");
  while (readRoom(file, &room)) {
    if (room.isAvailable)
    {
      printf("Room #%03d | %-20s | %d bed(s) | PHP ",
             room.roomNumber, room.category, room.bedrooms);
      printWithCommas(room.pricePerNight);
      printf("/night\n");
      foundAny = 1;
    }
  }

  if (!foundAny)
    printf("No vacant rooms currently.\n");
  fclose(file);
}

// ─── Option 2: Reserve ────────────────────────────────────────────────────────

void reserve() {
    FILE *file = fopen("rooms.txt", "r");
    if (!file) { printf("Could not open rooms.txt\n"); return; }

    Room rooms[MAX_ROOMS];
    int  roomCount = 0;
    while (readRoom(file, &rooms[roomCount])) roomCount++;
    fclose(file);

    // ── Gather guest info ─────────────────────────────────────────────────────
    char checkIn[20], checkOut[20], guestName[50];
    int  numberOfGuests, numberOfDays;

    printf("\nCheck-In  (e.g. February 25, 2024): ");
    fgets(checkIn, sizeof(checkIn), stdin);
    checkIn[strcspn(checkIn, "\n")] = '\0';

    printf("Check-Out (e.g. February 28, 2024): ");
    fgets(checkOut, sizeof(checkOut), stdin);
    checkOut[strcspn(checkOut, "\n")] = '\0';

    printf("Guest Name   : ");
    fgets(guestName, sizeof(guestName), stdin);
    guestName[strcspn(guestName, "\n")] = '\0';

    printf("No. of Guests: ");
    scanf("%d", &numberOfGuests);
    while (getchar() != '\n');

    printf("No. of Days  : ");
    scanf("%d", &numberOfDays);
    while (getchar() != '\n');

    // ── Room type selection ───────────────────────────────────────────────────
    printf("\nRoom Type:\n");
    printf("  [A] - De Luxe\n");
    printf("  [B] - Suite\n");
    printf("  [C] - Luxury Suite\n");
    printf("Choice: ");
    char roomTypePick;
    scanf("%c", &roomTypePick);
    while (getchar() != '\n');
    roomTypePick = toupper(roomTypePick);

    const char *chosenCategory;
    switch (roomTypePick) {
        case 'A': chosenCategory = "De Luxe";      break;
        case 'B': chosenCategory = "Suite";         break;
        case 'C': chosenCategory = "Luxury Suite";  break;
        default:
            printf("Invalid room type choice.\n");
            return;
    }

    // ── Filter and display vacant rooms of chosen category ────────────────────
    printf("\n--- AVAILABLE %s ROOMS ---\n", chosenCategory);
    int anyMatchingVacant = 0;
    for (int i = 0; i < roomCount; i++) {
        if (rooms[i].isAvailable &&
            strcmp(rooms[i].category, chosenCategory) == 0) {
            printf("Room #%03d | %d bed(s) | PHP ",
                   rooms[i].roomNumber, rooms[i].bedrooms);
            printWithCommas(rooms[i].pricePerNight);
            printf("/night\n");
            anyMatchingVacant = 1;
        }
    }

    if (!anyMatchingVacant) {
        printf("No vacant %s rooms available.\n", chosenCategory);
        return;
    }

    // ── Ask preferred room number, re-ask on invalid ──────────────────────────
    int selectedIndex = -1;
    do {
        int roomPick;
        printf("\nEnter preferred room number: ");
        scanf("%d", &roomPick);
        while (getchar() != '\n');

        for (int i = 0; i < roomCount; i++) {
            if (rooms[i].roomNumber   == roomPick &&
                rooms[i].isAvailable  == 1 &&
                strcmp(rooms[i].category, chosenCategory) == 0) {
                selectedIndex = i;
                break;
            }
        }

        if (selectedIndex == -1)
            printf("Invalid room number. Please choose from the list above.\n");

    } while (selectedIndex == -1);

    // ── Compute totals ────────────────────────────────────────────────────────
    float roomRate = rooms[selectedIndex].pricePerNight * numberOfDays;

    char referenceNumber[10];
    generateReferenceNumber(referenceNumber);

    // ── Reservation summary ───────────────────────────────────────────────────
    printf("\nRoom Type   : %s\n", chosenCategory);
    printf("Price/night : PHP "); printWithCommas(rooms[selectedIndex].pricePerNight); printf("\n");
    printf("Room Rate   : PHP "); printWithCommas(roomRate);
    printf(" (PHP "); printWithCommas(rooms[selectedIndex].pricePerNight);
    printf(" x %d days)\n", numberOfDays);
    printf("Your reference no: %s\n", referenceNumber);

    // ── Confirm proceed ───────────────────────────────────────────────────────
    printf("\nDo you want to proceed? [y/n]: ");
    char confirmReservation;
    scanf("%c", &confirmReservation);
    while (getchar() != '\n');

    if (tolower(confirmReservation) != 'y') {
        printf("Reservation cancelled. Returning to main menu.\n");
        return;
    }

    // ── Mark room as occupied and update rooms.txt ────────────────────────────
    rooms[selectedIndex].isAvailable = 0;
    file = fopen("rooms.txt", "w");
    if (!file) { printf("Could not update rooms.txt\n"); return; }
    for (int i = 0; i < roomCount; i++) {
        fprintf(file, "Room #%03d:\n",  rooms[i].roomNumber);
        fprintf(file, "Category: %s\n", rooms[i].category);
        fprintf(file, "Bedrooms: %d\n", rooms[i].bedrooms);
        fprintf(file, "Price: %.2f\n",  rooms[i].pricePerNight);
        fprintf(file, "Status: %s\n",   rooms[i].isAvailable ? "Vacant" : "Occupied");
        fprintf(file, "\n");
    }
    fclose(file);

    // ── Append to bookings.txt ────────────────────────────────────────────────
    file = fopen("bookings.txt", "a");
    if (!file) { printf("Could not open bookings.txt\n"); return; }
    fprintf(file, "\n");
    fprintf(file, "------- Guest Info -------\n");
    fprintf(file, "Reference No: %s\n",   referenceNumber);
    fprintf(file, "Room #: %03d\n",       rooms[selectedIndex].roomNumber);
    fprintf(file, "Room Type: %s\n",      chosenCategory);
    fprintf(file, "Main Guest: %s\n",     guestName);
    fprintf(file, "No of Guest: %d\n",    numberOfGuests);
    fprintf(file, "Check-In: %s\n",       checkIn);
    fprintf(file, "Checkout: %s\n",       checkOut);
    fprintf(file, "No of Days: %d\n",     numberOfDays);
    fprintf(file, "\n");
    fprintf(file, "------- Bill Info -------\n");
    fprintf(file, "Room Rate: %.2f\n",    roomRate);
    fprintf(file, "Amenities Used:\n");
    fprintf(file, "Amenities Total: 0.00\n");
    fprintf(file, "Final Amount: %.2f\n", roomRate);
    fprintf(file, "\n");
    fprintf(file, "------- Payment Info -------\n");
    fprintf(file, "Status: Not Paid\n");
    fprintf(file, "Method:\n");
    fprintf(file, "Amount Received:\n");
    fprintf(file, "Change:\n");
    fprintf(file, "\n");
    fprintf(file, "===========================================================\n");
    fclose(file);

    // ── Display booking confirmation ──────────────────────────────────────────
    printf("\nBooking\n");
    printf("Input reference no. %s\n", referenceNumber);
    printf("Details:\n");
    printf("   Check-In          : %s\n",   checkIn);
    printf("   Check-Out         : %s\n",   checkOut);
    printf("   Name of Main Guest: %s\n",   guestName);
    printf("   Number of Guests  : %d\n",   numberOfGuests);
    printf("   No. of Days       : %d\n",   numberOfDays);
    printf("   Room Type         : %s\n",   chosenCategory);
    printf("   Price/night       : PHP ");  printWithCommas(rooms[selectedIndex].pricePerNight); printf("\n");
    printf("   Room Rate         : PHP ");  printWithCommas(roomRate); printf("\n");

    // ── Offer payment ─────────────────────────────────────────────────────────
    printf("\nDo you want to proceed to payment? [y/n]: ");
    char payNow;
    scanf("%c", &payNow);
    while (getchar() != '\n');

    if (tolower(payNow) == 'y')
        payment(referenceNumber, 1);
    else
        printf("You can pay later using option 3.\n");
}

// ─── Option 3: Payment ────────────────────────────────────────────────────────

void payment(const char *referenceNumber, int fromReserve) {
  Reservation reservation;

  if (!findBooking(referenceNumber, &reservation)) {
    printf("Booking %s not found.\n", referenceNumber);
    return;
  }

  if (reservation.isPaid) {
    printf("Booking %s is already paid.\n", referenceNumber);
    return;
  }

  // Amenities — only when coming straight from reserve()
  if (fromReserve) {
    Amenity selectedAmenities[MAX_AMENITIES];
    int selectedCount = 0;
    char addMore = 'y';

    while (tolower(addMore) == 'y') {
      printf("\n--- AMENITIES ---\n");
      printf("  [A] - Convenience\n");
      printf("  [B] - Pool\n");
      printf("  [C] - Spa\n");
      printf("  [N] - No Amenities\n");
      printf("Choice: ");
      char categoryPick;
      scanf("%c", &categoryPick);
      while (getchar() != '\n');
      categoryPick = toupper(categoryPick);

      if (categoryPick == 'N')
        break;

      char filepath[60];
      switch (categoryPick) {
      case 'A':
        strcpy(filepath, "Amenities/convenienceAmenite.txt");
        break;
      case 'B':
        strcpy(filepath, "Amenities/poolAmenite.txt");
        break;
      case 'C':
        strcpy(filepath, "Amenities/spaAmenite.txt");
        break;
      default:
        printf("Invalid choice.\n");
        continue;
      }

      Amenity availableAmenities[10];
      int availableCount = readAmenities(filepath, availableAmenities, 10);

      if (availableCount == 0) {
        printf("No amenities found in file.\n");
        continue;
      }

      // Display amenities table
      printf("\n%-6s | %-40s | %-12s | %s\n", "Code", "Name", "Price", "Type");
      printf("--------------------------------------------------------------------\n");
      for (int i = 0; i < availableCount; i++) {
        printf("%-6s | %-40s | PHP ",
               availableAmenities[i].code,
               availableAmenities[i].name);
        printWithCommas(availableAmenities[i].price);
        printf(" | %s\n", availableAmenities[i].type);
      }

      // Pick by code
      printf("\nEnter code to add (or 0 to skip): ");
      char codePick[5];
      scanf("%4s", codePick);
      while (getchar() != '\n');

      if (strcmp(codePick, "0") == 0)
        goto askMore;

      int foundIndex = -1;
      for (int i = 0; i < availableCount; i++) {
        if (strcasecmp(availableAmenities[i].code, codePick) == 0)
        {
          foundIndex = i;
          break;
        }
      }

      if (foundIndex == -1) {
        printf("Code not found.\n");
        goto askMore;
      }

      // Calculate cost based on type
      float amenityCost = 0;
      if (strcmp(availableAmenities[foundIndex].type, "PerNight") == 0)
        amenityCost = availableAmenities[foundIndex].price * reservation.numberOfDays;
      else
        amenityCost = availableAmenities[foundIndex].price * reservation.numberOfGuests;

      reservation.amenitiesTotal += amenityCost;

      if (selectedCount < MAX_AMENITIES)
        selectedAmenities[selectedCount++] = availableAmenities[foundIndex];

      printf("Added: %s — PHP ", availableAmenities[foundIndex].name);
      printWithCommas(amenityCost);
      printf("\n");

    askMore:
      printf("Add another amenity? [y/n]: ");
      scanf("%c", &addMore);
      while (getchar() != '\n')
        ;
    }

    // Write amenity codes to bookings.txt
    if (selectedCount > 0) {
      FILE *file = fopen("bookings.txt", "r");
      if (file) {
        char lines[MAX_BOOKINGS][200];
        int totalLines = 0;
        while (totalLines < MAX_BOOKINGS &&
               fgets(lines[totalLines], sizeof(lines[totalLines]), file)) {
          lines[totalLines][strcspn(lines[totalLines], "\n")] = '\0';
          totalLines++;
        }
        fclose(file);

        int inTargetBlock = 0;
        for (int i = 0; i < totalLines; i++) {
          if (strncmp(lines[i], "Reference No:", 13) == 0) {
            char tmp[10];
            sscanf(lines[i], "Reference No: %9s", tmp);
            inTargetBlock = (strcmp(tmp, reservation.referenceNumber) == 0);
          }

          if (inTargetBlock && strncmp(lines[i], "Amenities Used:", 15) == 0) {
            char amenityLine[200] = "Amenities Used:";
            for (int j = 0; j < selectedCount; j++) {
              strcat(amenityLine, " ");
              strcat(amenityLine, selectedAmenities[j].code);
              if (j < selectedCount - 1)
                strcat(amenityLine, ",");
            }
            strcpy(lines[i], amenityLine);
          }
          if (strncmp(lines[i], "===========", 11) == 0)
            inTargetBlock = 0;
        }

        file = fopen("bookings.txt", "w");
        if (file) {
          for (int i = 0; i < totalLines; i++)
            fprintf(file, "%s\n", lines[i]);
          fclose(file);
        }
      }
    }
  }

  reservation.finalAmount = reservation.roomRate + reservation.amenitiesTotal;

  // Bill breakdown
  printf("\n========================================\n");
  printf("           BILL BREAKDOWN\n");
  printf("========================================\n");
  printf("Reference No  : %s\n", reservation.referenceNumber);
  printf("Main Guest    : %s\n", reservation.guestName);
  printf("Check-In      : %s\n", reservation.checkIn);
  printf("Check-Out     : %s\n", reservation.checkOut);
  printf("No. of Days   : %d\n", reservation.numberOfDays);
  printf("Room Type     : %s\n", reservation.roomType);
  printf("----------------------------------------\n");
  printf("Room Rate     : PHP ");
  printWithCommas(reservation.roomRate);
  printf("\n");
  printf("Amenities     : PHP ");
  printWithCommas(reservation.amenitiesTotal);
  printf("\n");
  printf("----------------------------------------\n");
  printf("TOTAL DUE     : PHP ");
  printWithCommas(reservation.finalAmount);
  printf("\n");
  printf("========================================\n");

  printf("\nDo you want to proceed with payment? [y/n]: ");
  char confirmPayment;
  scanf("%c", &confirmPayment);
  while (getchar() != '\n')
    ;

  if (tolower(confirmPayment) != 'y') {
    printf("Payment cancelled. Returning to main menu.\n");
    return;
  }

  printf("Amount Received: PHP ");
  scanf("%f", &reservation.amountReceived);
  while (getchar() != '\n')
    ;

  if (reservation.amountReceived < reservation.finalAmount) {
    printf("Insufficient amount. Payment not processed.\n");
    return;
  }

  reservation.change = reservation.amountReceived - reservation.finalAmount;
  reservation.isPaid = 1;
  strcpy(reservation.paymentMethod, "Cash");

  printf("\nChange        : PHP ");
  printWithCommas(reservation.change);
  printf("\n");
  printf("Payment successful! Thank you, %s.\n", reservation.guestName);

  updateBookingPayment(&reservation);
}

// ─── Option 4: Registry ───────────────────────────────────────────────────────

void registry() {
  char searchName[50];
  char searchAgain;

  do{
    printf("\n--- REGISTRY ---\n");
    printf("Enter first name to search: ");
    fgets(searchName, sizeof(searchName), stdin);
    searchName[strcspn(searchName, "\n")] = '\0';

    FILE *file = fopen("bookings.txt", "r");
    if (!file) {
      printf("Could not open bookings.txt\n");
      return;
    }

    char line[200];
    char blockBuffer[20][200];
    int blockLineCount = 0;
    int insideBlock = 0;
    int anyFound = 0;

    while (fgets(line, sizeof(line), file)) {
      line[strcspn(line, "\n")] = '\0';

      if (strncmp(line, "------- Guest Info", 18) == 0) {
        insideBlock = 1;
        blockLineCount = 0;
        memset(blockBuffer, 0, sizeof(blockBuffer));
      }

      if (insideBlock && blockLineCount < 20)
        strcpy(blockBuffer[blockLineCount++], line);

      if (strncmp(line, "Main Guest:", 11) == 0) {
        char fullName[50] = "";
        char firstName[50] = "";
        sscanf(line, "Main Guest: %49[^\n]", fullName);
        sscanf(fullName, "%49s", firstName);

        if (strcasecmp(firstName, searchName) == 0)
        {
          anyFound = 1;
          printf("\n--- BOOKING FOUND ---\n");
          for (int i = 0; i < blockLineCount; i++)
            printf("%s\n", blockBuffer[i]);
        }
      }

      if (strncmp(line, "===========", 11) == 0 && insideBlock)  {
        if (anyFound)
          printf("%s\n", line);
        insideBlock = 0;
        blockLineCount = 0;
        anyFound = 0;
      }
    }

    fclose(file);
    if (!anyFound)
      printf("No bookings found for \"%s\".\n", searchName);

    printf("\nSearch another guest? [y/n]: ");
    scanf("%c", &searchAgain);
    while (getchar() != '\n')
      ;

  } while (tolower(searchAgain) == 'y');

  printf("Returning to main menu.\n");
}

// ─── Option 5: View Room Details ──────────────────────────────────────────────

void viewDetails() {
  FILE *file = fopen("rooms.txt", "r");
  if (!file) {
    printf("Could not open rooms.txt\n");
    return;
  }

  Room rooms[MAX_ROOMS];
  int roomCount = 0;
  while (readRoom(file, &rooms[roomCount]))
    roomCount++;
  fclose(file);

  printf("\n--- ALL ROOMS ---\n");
  for (int i = 0; i < roomCount; i++) {
    printf("Room #%03d | %-20s | %s\n",
           rooms[i].roomNumber, rooms[i].category,
           rooms[i].isAvailable ? "Vacant" : "Occupied");
  }

  int roomPick;
  printf("\nEnter room number to view: ");
  scanf("%d", &roomPick);
  while (getchar() != '\n')
    ;

  int selectedIndex = -1;
  for (int i = 0; i < roomCount; i++) {
    if (rooms[i].roomNumber == roomPick) {
      selectedIndex = i;
      break;
    }
  }

  if (selectedIndex == -1) {
    printf("Room #%03d not found.\n", roomPick);
    return;
  }

  Room selected = rooms[selectedIndex];
  printf("\n--- ROOM DETAILS ---\n");
  printf("Room     : #%03d\n", selected.roomNumber);
  printf("Category : %s\n", selected.category);
  printf("Bedrooms : %d\n", selected.bedrooms);
  printf("Price    : PHP ");
  printWithCommas(selected.pricePerNight);
  printf("/night\n");
  printf("Status   : %s\n", selected.isAvailable ? "Vacant" : "Occupied");
}




// ─── UTILS FUNCTIONS ──────────────────────────────────────────────────────────

void printWithCommas(float amount) {
  int wholeNumber = (int)amount;
  int centsPart = (int)((amount - wholeNumber) * 100 + 0.5f);

  if (wholeNumber >= 1000000)
    printf("%d,%03d,%03d.%02d",
           wholeNumber / 1000000,
           (wholeNumber / 1000) % 1000,
           wholeNumber % 1000,
           centsPart);
  else if (wholeNumber >= 1000)
    printf("%d,%03d.%02d",
           wholeNumber / 1000,
           wholeNumber % 1000,
           centsPart);
  else
    printf("%d.%02d", wholeNumber, centsPart);
}

void generateReferenceNumber(char *output) {
  int existingCount = 0;
  FILE *file = fopen("bookings.txt", "r");
  if (file) {
    char line[200];
    while (fgets(line, sizeof(line), file))
      if (strncmp(line, "Reference No:", 13) == 0)
        existingCount++;
    fclose(file);
  }
  sprintf(output, "B%04d", existingCount + 1);
}

int readRoom(FILE *fp, Room *room) {
  char line[150];
  int fieldsFilled = 0;
  memset(room, 0, sizeof(Room));

  while (fgets(line, sizeof(line), fp)) {
    line[strcspn(line, "\n")] = '\0';

    if (strncmp(line, "Room #", 6) == 0) {
      sscanf(line, "Room #%d:", &room->roomNumber);
      fieldsFilled++;
    }
    else if (strncmp(line, "Category:", 9) == 0) {
      sscanf(line, "Category: %29[^\n]", room->category);
      fieldsFilled++;
    }
    else if (strncmp(line, "Bedrooms:", 9) == 0) {
      sscanf(line, "Bedrooms: %d", &room->bedrooms);
      fieldsFilled++;
    }
    else if (strncmp(line, "Price:", 6) == 0) {
      sscanf(line, "Price: %f", &room->pricePerNight);
      fieldsFilled++;
    }
    else if (strncmp(line, "Status:", 7) == 0) {
      room->isAvailable = (strstr(line, "Vacant") != NULL);
      fieldsFilled++;
    }

    if (line[0] == '\0' && fieldsFilled > 0)
      break;
  }

  return fieldsFilled > 0;
}

int readAmenities(const char *filename, Amenity *list, int maxCount) {
  FILE *file = fopen(filename, "r");
  if (!file)
    return 0;

  char line[150];
  int count = 0;
  Amenity *currentItem = &list[count];

  while (fgets(line, sizeof(line), file) && count < maxCount) {
    line[strcspn(line, "\n")] = '\0';

    if (strncmp(line, "Code:", 5) == 0) {
      currentItem = &list[count];
      memset(currentItem, 0, sizeof(Amenity));
      sscanf(line, "Code: %4s", currentItem->code);
    }
    else if (strncmp(line, "Name:", 5) == 0)
      sscanf(line, "Name: %59[^\n]", currentItem->name);
    else if (strncmp(line, "Price:", 6) == 0)
      sscanf(line, "Price: %f", &currentItem->price);
    else if (strncmp(line, "Type:", 5) == 0) {
      sscanf(line, "Type: %19[^\n]", currentItem->type);
      count++;
    }
  }

  fclose(file);
  return count;
}

int findBooking(const char *referenceNumber, Reservation *reservation) {
  FILE *file = fopen("bookings.txt", "r");
  if (!file) {
    printf("Could not open bookings.txt\n");
    return 0;
  }

  char line[200];
  int insideBlock = 0;
  int found = 0;

  while (fgets(line, sizeof(line), file)) {
    line[strcspn(line, "\n")] = '\0';

    if (strncmp(line, "------- Guest Info", 18) == 0) {
      insideBlock = 1;
      memset(reservation, 0, sizeof(Reservation));
    }

    if (!insideBlock)
      continue;

    if (strncmp(line, "Reference No:", 13) == 0)
      sscanf(line, "Reference No: %9s", reservation->referenceNumber);
    else if (strncmp(line, "Room Type:", 10) == 0)
      sscanf(line, "Room Type: %29[^\n]", reservation->roomType);
    else if (strncmp(line, "Main Guest:", 11) == 0)
      sscanf(line, "Main Guest: %49[^\n]", reservation->guestName);
    else if (strncmp(line, "No of Guest:", 12) == 0)
      sscanf(line, "No of Guest: %d", &reservation->numberOfGuests);
    else if (strncmp(line, "Check-In:", 9) == 0)
      sscanf(line, "Check-In: %19[^\n]", reservation->checkIn);
    else if (strncmp(line, "Checkout:", 9) == 0)
      sscanf(line, "Checkout: %19[^\n]", reservation->checkOut);
    else if (strncmp(line, "No of Days:", 11) == 0)
      sscanf(line, "No of Days: %d", &reservation->numberOfDays);
    else if (strncmp(line, "Room Rate:", 10) == 0)
      sscanf(line, "Room Rate: %f", &reservation->roomRate);
    else if (strncmp(line, "Amenities Total:", 16) == 0)
      sscanf(line, "Amenities Total: %f", &reservation->amenitiesTotal);
    else if (strncmp(line, "Final Amount:", 13) == 0)
      sscanf(line, "Final Amount: %f", &reservation->finalAmount);
    else if (strncmp(line, "Status:", 7) == 0)
      reservation->isPaid = (strstr(line, "Not Paid") == NULL &&
                             strstr(line, "Paid") != NULL);

    if (strncmp(line, "===========", 11) == 0 && insideBlock) {
      insideBlock = 0;
      if (strcmp(reservation->referenceNumber, referenceNumber) == 0) {
        found = 1;
        break;
      }
    }
  }

  fclose(file);
  return found;
}

void updateBookingPayment(Reservation *reservation) {
  FILE *file = fopen("bookings.txt", "r");
  if (!file) { printf("Could not open bookings.txt\n"); return; }

  char lines[MAX_BOOKINGS][200];
  int totalLines = 0;
  while (totalLines < MAX_BOOKINGS &&
         fgets(lines[totalLines], sizeof(lines[totalLines]), file))
  {
    lines[totalLines][strcspn(lines[totalLines], "\n")] = '\0';
    totalLines++;
  }
  fclose(file);

  int inTargetBlock = 0;
  for (int i = 0; i < totalLines; i++) {
    if (strncmp(lines[i], "Reference No:", 13) == 0) {
      char tmp[10];
      sscanf(lines[i], "Reference No: %9s", tmp);
      inTargetBlock = (strcmp(tmp, reservation->referenceNumber) == 0);
    }

    if (!inTargetBlock)
      continue;

    if (strncmp(lines[i], "Amenities Total:", 16) == 0)
      sprintf(lines[i], "Amenities Total: %.2f", reservation->amenitiesTotal);
    else if (strncmp(lines[i], "Final Amount:", 13) == 0)
      sprintf(lines[i], "Final Amount: %.2f", reservation->finalAmount);
    else if (strncmp(lines[i], "Status:", 7) == 0)
      sprintf(lines[i], "Status: %s", reservation->isPaid ? "Paid" : "Not Paid");
    else if (strncmp(lines[i], "Method:", 7) == 0)
      sprintf(lines[i], "Method: %s", reservation->paymentMethod);
    else if (strncmp(lines[i], "Amount Received:", 16) == 0)
      sprintf(lines[i], "Amount Received: %.2f", reservation->amountReceived);
    else if (strncmp(lines[i], "Change:", 7) == 0)
      sprintf(lines[i], "Change: %.2f", reservation->change);

    if (strncmp(lines[i], "===========", 11) == 0)
      inTargetBlock = 0;
  }

  file = fopen("bookings.txt", "w");
  if (!file) {
    printf("Could not save payment.\n");
    return;
  }

  for (int i = 0; i < totalLines; i++)
    fprintf(file, "%s\n", lines[i]);
  fclose(file);
}