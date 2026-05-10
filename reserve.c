#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ROOMS     100
#define MAX_BOOKINGS  500
#define MAX_AMENITIES 20

// ─── Structs ───────────────────────────────────────────────────────────────────

typedef struct {
  char  referenceNumber[10];
  char  guestName[50];
  char  checkIn[20];
  char  checkOut[20];
  int   numberOfGuests;
  int   numberOfDays;
  char  roomType[30];
  int   roomNumber;          // FIX #5: now parsed in findBooking()
  float pricePerNight;
  float roomRate;
  float amenitiesTotal;
  float finalAmount;
  float amountReceived;
  float change;
  char  paymentMethod[20];
  int   isPaid;
} Reservation;

typedef struct {
  int   roomNumber;
  char  category[30];
  int   bedrooms;
  float pricePerNight;
  int   isAvailable;
} Room;

typedef struct {
  char  code[5];
  char  name[60];
  float price;
  char  type[20];
} Amenity;

// ─── Forward Declarations ─────────────────────────────────────────────────────

void displayMenu();
void listVacant();
void reserve();
void payment(const char *referenceNumber);   // FIX #8: removed fromReserve param
void registry();
void viewDetails();
void deleteBooking();                         // NEW
void checkout();                              // FIX #10: implemented

void generateReferenceNumber(char *output);
int  readRoom(FILE *fp, Room *room);
int  readAmenities(const char *filename, Amenity *list, int maxCount);
int  findBooking(const char *referenceNumber, Reservation *reservation);
void updateBookingPayment(Reservation *reservation);
void markRoomVacant(int roomNumber);          // NEW
void removeBookingFromFile(const char *referenceNumber); // NEW
void generateReceipt(Reservation *reservation);
int  isValidDate(const char *date);           // FIX #3
void printWithCommas(float amount);

// ─── Main ──────────────────────────────────────────────────────────────────────

int main() {
  int choice;
  do {
    displayMenu();
    printf("Selection: ");
    if (scanf("%d", &choice) == 0) {
      while (getchar() != '\n');
      printf("Invalid input. Please enter a number.\n");
      continue;
    }
    while (getchar() != '\n');

    switch (choice) {
      case 1: listVacant(); break;
      case 2: reserve();    break;
      case 3: {
        char refInput[10];
        printf("Enter Reference No: ");
        scanf("%9s", refInput);
        while (getchar() != '\n');
        payment(refInput);
        break;
      }
      case 4: registry();      break;
      case 5: viewDetails();   break;
      case 6: deleteBooking(); break;
      case 7: checkout();      break;
      case 8: printf("Exiting ESPLENIN HOTEL system...\n"); break;
      default: printf("Invalid choice. Try again.\n");
    }
  } while (choice != 8);

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
  printf("6. Delete Booking\n");
  printf("7. Checkout\n");
  printf("8. Exit\n");
  printf("============================\n");
}

// ─── Option 1: List Vacant Rooms ──────────────────────────────────────────────

void listVacant() {
  FILE *file = fopen("rooms.txt", "r");
  if (!file) { printf("Could not open rooms.txt\n"); return; }

  Room room;
  int foundAny = 0;

  printf("\n--- VACANT ROOMS ---\n");
  while (readRoom(file, &room)) {
    if (room.isAvailable) {
      printf("Room #%03d | %-20s | %d bed(s) | PHP ",
             room.roomNumber, room.category, room.bedrooms);
      printWithCommas(room.pricePerNight);
      printf("/night\n");
      foundAny = 1;
    }
  }
  if (!foundAny) printf("No vacant rooms currently.\n");
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

  char checkIn[20], checkOut[20], guestName[50];
  int  numberOfGuests, numberOfDays;

  // FIX #3: Date validation loops
  do {
    printf("\nCheck-In  (e.g. March 25, 2027): ");
    fgets(checkIn, sizeof(checkIn), stdin);
    checkIn[strcspn(checkIn, "\n")] = '\0';
    if (!isValidDate(checkIn))
      printf("Invalid format. Use: Month Day, Year (e.g. March 25, 2027)\n");
  } while (!isValidDate(checkIn));

  do {
    printf("Check-Out (e.g. March 29, 2027): ");
    fgets(checkOut, sizeof(checkOut), stdin);
    checkOut[strcspn(checkOut, "\n")] = '\0';
    if (!isValidDate(checkOut))
      printf("Invalid format. Use: Month Day, Year (e.g. March 29, 2027)\n");
  } while (!isValidDate(checkOut));

  printf("Guest Name   : ");
  fgets(guestName, sizeof(guestName), stdin);
  guestName[strcspn(guestName, "\n")] = '\0';

  // Validate positive numbers
  do {
    printf("No. of Guests: ");
    scanf("%d", &numberOfGuests);
    while (getchar() != '\n');
    if (numberOfGuests <= 0) printf("Must be at least 1.\n");
  } while (numberOfGuests <= 0);

  do {
    printf("No. of Days  : ");
    scanf("%d", &numberOfDays);
    while (getchar() != '\n');
    if (numberOfDays <= 0) printf("Must be at least 1.\n");
  } while (numberOfDays <= 0);

  // Room type selection
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

  // Filter and display vacant rooms of chosen category
  printf("\n--- AVAILABLE %s ROOMS ---\n", chosenCategory);
  int anyMatchingVacant = 0;
  for (int i = 0; i < roomCount; i++) {
    if (rooms[i].isAvailable && strcmp(rooms[i].category, chosenCategory) == 0) {
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

  // Ask preferred room number, re-ask on invalid
  int selectedIndex = -1;
  do {
    int roomPick;
    printf("\nEnter preferred room number: ");
    scanf("%d", &roomPick);
    while (getchar() != '\n');

    for (int i = 0; i < roomCount; i++) {
      if (rooms[i].roomNumber  == roomPick &&
          rooms[i].isAvailable == 1 &&
          strcmp(rooms[i].category, chosenCategory) == 0) {
        selectedIndex = i;
        break;
      }
    }
    if (selectedIndex == -1)
      printf("Invalid room number. Please choose from the list above.\n");
  } while (selectedIndex == -1);

  float roomRate = rooms[selectedIndex].pricePerNight * numberOfDays;
  char  referenceNumber[10];
  generateReferenceNumber(referenceNumber);

  // Reservation summary
  printf("\nRoom Type   : %s\n", chosenCategory);
  printf("Price/night : PHP "); printWithCommas(rooms[selectedIndex].pricePerNight); printf("\n");
  printf("Room Rate   : PHP "); printWithCommas(roomRate);
  printf(" (PHP "); printWithCommas(rooms[selectedIndex].pricePerNight);
  printf(" x %d days)\n", numberOfDays);
  printf("Reference No: %s\n", referenceNumber);

  printf("\nDo you want to proceed? [y/n]: ");
  char confirmReservation;
  scanf("%c", &confirmReservation);
  while (getchar() != '\n');

  if (tolower(confirmReservation) != 'y') {
    printf("Reservation cancelled.\n");
    return;
  }

  // Mark room as occupied and update rooms.txt
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

  // Append to bookings.txt
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

  // Booking confirmation
  printf("\nBooking confirmed!\n");
  printf("   Reference No      : %s\n",   referenceNumber);
  printf("   Check-In          : %s\n",   checkIn);
  printf("   Check-Out         : %s\n",   checkOut);
  printf("   Name of Main Guest: %s\n",   guestName);
  printf("   Number of Guests  : %d\n",   numberOfGuests);
  printf("   No. of Days       : %d\n",   numberOfDays);
  printf("   Room Type         : %s\n",   chosenCategory);
  printf("   Price/night       : PHP ");   printWithCommas(rooms[selectedIndex].pricePerNight); printf("\n");
  printf("   Room Rate         : PHP ");   printWithCommas(roomRate); printf("\n");

  printf("\nDo you want to proceed to payment? [y/n]: ");
  char payNow;
  scanf("%c", &payNow);
  while (getchar() != '\n');

  if (tolower(payNow) == 'y')
    payment(referenceNumber);
  else
    printf("You can pay later using option 3.\n");
}

// ─── Option 3: Payment ────────────────────────────────────────────────────────
// FIX #8: Amenities are always offered, not just when coming from reserve()
// FIX #4: Payment method is now chosen by the user (Cash / Card / GCash)
// FIX #1: goto removed — restructured with flag

void payment(const char *referenceNumber) {
  Reservation reservation;

  if (!findBooking(referenceNumber, &reservation)) {
    printf("Booking %s not found.\n", referenceNumber);
    return;
  }

  if (reservation.isPaid) {
    printf("Booking %s is already paid.\n", referenceNumber);
    return;
  }

  // ── Amenities selection ──────────────────────────────────────────────────
  Amenity selectedAmenities[MAX_AMENITIES];
  int selectedCount = 0;
  char addMore;

  printf("\nWould you like to add amenities? [y/n]: ");
  scanf("%c", &addMore);
  while (getchar() != '\n');

  while (tolower(addMore) == 'y') {
    printf("\n--- AMENITY CATEGORY ---\n");
    printf("  [A] - Convenience\n");
    printf("  [B] - Pool\n");
    printf("  [C] - Spa\n");
    printf("  [N] - Done adding\n");
    printf("Choice: ");
    char categoryPick;
    scanf("%c", &categoryPick);
    while (getchar() != '\n');
    categoryPick = toupper(categoryPick);

    if (categoryPick == 'N') break;

    char filepath[60];
    int validCategory = 1;
    switch (categoryPick) {
      case 'A': strcpy(filepath, "Amenities/convenienceAmenite.txt"); break;
      case 'B': strcpy(filepath, "Amenities/poolAmenite.txt");        break;
      case 'C': strcpy(filepath, "Amenities/spaAmenite.txt");         break;
      default:
        printf("Invalid choice.\n");
        validCategory = 0;
    }

    if (validCategory) {
      Amenity availableAmenities[10];
      int availableCount = readAmenities(filepath, availableAmenities, 10);

      if (availableCount == 0) {
        printf("No amenities found.\n");
      } else {
        printf("\n%-6s | %-40s | %-12s | %s\n", "Code", "Name", "Price", "Type");
        printf("--------------------------------------------------------------------\n");
        for (int i = 0; i < availableCount; i++) {
          printf("%-6s | %-40s | PHP ",
                 availableAmenities[i].code, availableAmenities[i].name);
          printWithCommas(availableAmenities[i].price);
          printf(" | %s\n", availableAmenities[i].type);
        }

        printf("\nEnter code to add (or 0 to skip): ");
        char codePick[5];
        scanf("%4s", codePick);
        while (getchar() != '\n');

        // FIX #1: replaced goto with an if block — no more goto!
        if (strcmp(codePick, "0") != 0) {
          int foundIndex = -1;
          for (int i = 0; i < availableCount; i++) {
            if (strcasecmp(availableAmenities[i].code, codePick) == 0) {
              foundIndex = i;
              break;
            }
          }

          if (foundIndex == -1) {
            printf("Code not found.\n");
          } else {
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
          }
        }
      }
    }

    printf("Add another amenity? [y/n]: ");
    scanf("%c", &addMore);
    while (getchar() != '\n');
  }

  // Write amenity codes to bookings.txt
  if (selectedCount > 0) {
    // FIX #6: use malloc instead of huge stack array
    char (*lines)[200] = malloc(MAX_BOOKINGS * sizeof(*lines));
    if (!lines) { printf("Memory error.\n"); return; }

    FILE *file = fopen("bookings.txt", "r");
    if (file) {
      int totalLines = 0;
      while (totalLines < MAX_BOOKINGS && fgets(lines[totalLines], 200, file)) {
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
          for (int j = 0; j < selectedCount && strlen(amenityLine) < 180; j++) {
            strcat(amenityLine, " ");
            strcat(amenityLine, selectedAmenities[j].code);
            if (j < selectedCount - 1) strcat(amenityLine, ",");
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
    free(lines);
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
  printf("Room Rate     : PHP "); printWithCommas(reservation.roomRate);    printf("\n");
  printf("Amenities     : PHP "); printWithCommas(reservation.amenitiesTotal); printf("\n");
  printf("----------------------------------------\n");
  printf("TOTAL DUE     : PHP "); printWithCommas(reservation.finalAmount); printf("\n");
  printf("========================================\n");

  printf("\nProceed with payment? [y/n]: ");
  char confirmPayment;
  scanf("%c", &confirmPayment);
  while (getchar() != '\n');

  if (tolower(confirmPayment) != 'y') {
    printf("Payment cancelled.\n");
    return;
  }

  // FIX #4: Ask payment method instead of hardcoding "Cash"
  printf("\nPayment Method:\n");
  printf("  [1] - Cash\n");
  printf("  [2] - Card\n");
  printf("  [3] - GCash\n");
  printf("Choice: ");
  int methodChoice;
  if (scanf("%d", &methodChoice) != 1) methodChoice = 1;
  while (getchar() != '\n');

  switch (methodChoice) {
    case 2:  strcpy(reservation.paymentMethod, "Card");  break;
    case 3:  strcpy(reservation.paymentMethod, "GCash"); break;
    default: strcpy(reservation.paymentMethod, "Cash");
  }

  printf("Amount Received: PHP ");
  scanf("%f", &reservation.amountReceived);
  while (getchar() != '\n');

  if (reservation.amountReceived < reservation.finalAmount) {
    printf("Insufficient amount. Payment not processed.\n");
    return;
  }

  reservation.change = reservation.amountReceived - reservation.finalAmount;
  reservation.isPaid = 1;

  printf("\nChange        : PHP "); printWithCommas(reservation.change); printf("\n");
  printf("Payment successful! Thank you, %s.\n", reservation.guestName);

  updateBookingPayment(&reservation);
  generateReceipt(&reservation);
}

// ─── Option 4: Registry ───────────────────────────────────────────────────────
// FIX #7: totalFound accumulates across all blocks so "not found" message is correct

void registry() {
  char searchName[50];
  char searchAgain;

  do {
    printf("\n--- REGISTRY ---\n");
    printf("Enter first name to search: ");
    fgets(searchName, sizeof(searchName), stdin);
    searchName[strcspn(searchName, "\n")] = '\0';

    FILE *file = fopen("bookings.txt", "r");
    if (!file) { printf("Could not open bookings.txt\n"); return; }

    char line[200];
    char blockBuffer[20][200];
    int  blockLineCount = 0;
    int  insideBlock    = 0;
    int  blockMatched   = 0;
    int  totalFound     = 0;  // FIX #7: survives across blocks

    while (fgets(line, sizeof(line), file)) {
      line[strcspn(line, "\n")] = '\0';

      if (strncmp(line, "------- Guest Info", 18) == 0) {
        insideBlock    = 1;
        blockLineCount = 0;
        blockMatched   = 0;
        memset(blockBuffer, 0, sizeof(blockBuffer));
      }

      if (insideBlock && blockLineCount < 20)
        strcpy(blockBuffer[blockLineCount++], line);

      if (strncmp(line, "Main Guest:", 11) == 0) {
        char fullName[50]  = "";
        char firstName[50] = "";
        sscanf(line, "Main Guest: %49[^\n]", fullName);
        sscanf(fullName, "%49s", firstName);

        if (strcasecmp(firstName, searchName) == 0) {
          blockMatched = 1;
          totalFound++;
          printf("\n--- BOOKING FOUND ---\n");
          for (int i = 0; i < blockLineCount; i++)
            printf("%s\n", blockBuffer[i]);
        }
      }

      if (strncmp(line, "===========", 11) == 0 && insideBlock) {
        if (blockMatched) printf("%s\n", line);
        insideBlock  = 0;
        blockMatched = 0;
      }
    }

    fclose(file);
    if (totalFound == 0)
      printf("No bookings found for \"%s\".\n", searchName);

    printf("\nSearch another guest? [y/n]: ");
    scanf("%c", &searchAgain);
    while (getchar() != '\n');

  } while (tolower(searchAgain) == 'y');
}

// ─── Option 5: View Room Details ──────────────────────────────────────────────

void viewDetails() {
  FILE *file = fopen("rooms.txt", "r");
  if (!file) { printf("Could not open rooms.txt\n"); return; }

  Room rooms[MAX_ROOMS];
  int  roomCount = 0;
  while (readRoom(file, &rooms[roomCount])) roomCount++;
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
  while (getchar() != '\n');

  int selectedIndex = -1;
  for (int i = 0; i < roomCount; i++) {
    if (rooms[i].roomNumber == roomPick) { selectedIndex = i; break; }
  }

  if (selectedIndex == -1) {
    printf("Room #%03d not found.\n", roomPick);
    return;
  }

  Room selected = rooms[selectedIndex];
  printf("\n--- ROOM DETAILS ---\n");
  printf("Room     : #%03d\n", selected.roomNumber);
  printf("Category : %s\n",    selected.category);
  printf("Bedrooms : %d\n",    selected.bedrooms);
  printf("Price    : PHP ");   printWithCommas(selected.pricePerNight); printf("/night\n");
  printf("Status   : %s\n",    selected.isAvailable ? "Vacant" : "Occupied");
}

// ─── Option 6: Delete Booking (NEW) ──────────────────────────────────────────
// Answers the question: Yes — deleting a booking via this option will
// automatically mark the room as Vacant in rooms.txt.

void deleteBooking() {
  char refInput[10];
  printf("\nEnter Reference No to delete: ");
  scanf("%9s", refInput);
  while (getchar() != '\n');

  Reservation reservation;
  if (!findBooking(refInput, &reservation)) {
    printf("Booking %s not found.\n", refInput);
    return;
  }

  printf("\n--- BOOKING TO DELETE ---\n");
  printf("Reference No : %s\n",  reservation.referenceNumber);
  printf("Guest        : %s\n",  reservation.guestName);
  printf("Room #       : %03d\n", reservation.roomNumber);
  printf("Room Type    : %s\n",  reservation.roomType);
  printf("Check-In     : %s\n",  reservation.checkIn);
  printf("Check-Out    : %s\n",  reservation.checkOut);
  printf("Status       : %s\n",  reservation.isPaid ? "Paid" : "Not Paid");

  printf("\nAre you sure you want to delete this booking? [y/n]: ");
  char confirm;
  scanf("%c", &confirm);
  while (getchar() != '\n');

  if (tolower(confirm) != 'y') {
    printf("Deletion cancelled.\n");
    return;
  }

  removeBookingFromFile(refInput);       // remove from bookings.txt
  markRoomVacant(reservation.roomNumber); // set room back to Vacant in rooms.txt

  printf("Booking %s deleted. Room #%03d is now Vacant.\n",
         refInput, reservation.roomNumber);
}

// ─── Option 7: Checkout (FIX #10 — implemented) ──────────────────────────────

void checkout() {
  char refInput[10];
  printf("\nEnter Reference No for checkout: ");
  scanf("%9s", refInput);
  while (getchar() != '\n');

  Reservation reservation;
  if (!findBooking(refInput, &reservation)) {
    printf("Booking %s not found.\n", refInput);
    return;
  }

  if (!reservation.isPaid) {
    printf("Booking %s is not yet paid. Please settle the bill first (Option 3).\n", refInput);
    return;
  }

  printf("\n--- CHECKOUT SUMMARY ---\n");
  printf("Reference No : %s\n",   reservation.referenceNumber);
  printf("Guest        : %s\n",   reservation.guestName);
  printf("Room #       : %03d\n", reservation.roomNumber);
  printf("Room Type    : %s\n",   reservation.roomType);
  printf("Check-In     : %s\n",   reservation.checkIn);
  printf("Check-Out    : %s\n",   reservation.checkOut);
  printf("Total Paid   : PHP ");  printWithCommas(reservation.finalAmount); printf("\n");

  printf("\nConfirm checkout? [y/n]: ");
  char confirm;
  scanf("%c", &confirm);
  while (getchar() != '\n');

  if (tolower(confirm) != 'y') {
    printf("Checkout cancelled.\n");
    return;
  }

  markRoomVacant(reservation.roomNumber);
  printf("\nCheckout successful! Room #%03d is now Vacant.\n", reservation.roomNumber);
  printf("Thank you for staying at ESPLENIN HOTEL, %s!\n", reservation.guestName);
}

// ─── UTILITY FUNCTIONS ────────────────────────────────────────────────────────

// FIX #3: Basic date format validator — expects "Month Day, Year"
int isValidDate(const char *date) {
  if (!date || strlen(date) < 8) return 0;
  if (!isalpha((unsigned char)date[0])) return 0; // must start with letter (month)
  if (strchr(date, ' ') == NULL) return 0;         // must have a space
  if (strchr(date, ',') == NULL) return 0;         // must have a comma
  // last character must be a digit (year)
  if (!isdigit((unsigned char)date[strlen(date) - 1])) return 0;
  return 1;
}

void printWithCommas(float amount) {
  int wholeNumber = (int)amount;
  int centsPart   = (int)((amount - wholeNumber) * 100 + 0.5f);

  if (wholeNumber >= 1000000)
    printf("%d,%03d,%03d.%02d",
           wholeNumber / 1000000,
           (wholeNumber / 1000) % 1000,
           wholeNumber % 1000,
           centsPart);
  else if (wholeNumber >= 1000)
    printf("%d,%03d.%02d", wholeNumber / 1000, wholeNumber % 1000, centsPart);
  else
    printf("%d.%02d", wholeNumber, centsPart);
}

// FIX #2: Find the MAX existing reference number instead of counting lines
void generateReferenceNumber(char *output) {
  int maxNumber = 0;
  FILE *file = fopen("bookings.txt", "r");
  if (file) {
    char line[200];
    while (fgets(line, sizeof(line), file)) {
      if (strncmp(line, "Reference No:", 13) == 0) {
        char refNum[10] = "";
        sscanf(line, "Reference No: %9s", refNum);
        if (refNum[0] == 'B') {
          int num = atoi(refNum + 1);
          if (num > maxNumber) maxNumber = num;
        }
      }
    }
    fclose(file);
  }
  sprintf(output, "B%04d", maxNumber + 1);
}

int readRoom(FILE *fp, Room *room) {
  char line[150];
  int  fieldsFilled = 0;
  memset(room, 0, sizeof(Room));

  while (fgets(line, sizeof(line), fp)) {
    line[strcspn(line, "\n")] = '\0';

    if      (strncmp(line, "Room #",    6) == 0) { sscanf(line, "Room #%d:",          &room->roomNumber);    fieldsFilled++; }
    else if (strncmp(line, "Category:", 9) == 0) { sscanf(line, "Category: %29[^\n]",  room->category);      fieldsFilled++; }
    else if (strncmp(line, "Bedrooms:", 9) == 0) { sscanf(line, "Bedrooms: %d",        &room->bedrooms);     fieldsFilled++; }
    else if (strncmp(line, "Price:",    6) == 0) { sscanf(line, "Price: %f",           &room->pricePerNight);fieldsFilled++; }
    else if (strncmp(line, "Status:",   7) == 0) { room->isAvailable = (strstr(line, "Vacant") != NULL);     fieldsFilled++; }

    if (line[0] == '\0' && fieldsFilled > 0) break;
  }

  return fieldsFilled > 0;
}

int readAmenities(const char *filename, Amenity *list, int maxCount) {
  FILE *file = fopen(filename, "r");
  if (!file) return 0;

  char    line[150];
  int     count       = 0;
  Amenity *currentItem = &list[0];

  while (fgets(line, sizeof(line), file) && count < maxCount) {
    line[strcspn(line, "\n")] = '\0';

    if      (strncmp(line, "Code:",  5) == 0) { currentItem = &list[count]; memset(currentItem, 0, sizeof(Amenity)); sscanf(line, "Code: %4s",     currentItem->code);  }
    else if (strncmp(line, "Name:",  5) == 0) { sscanf(line, "Name: %59[^\n]",  currentItem->name);  }
    else if (strncmp(line, "Price:", 6) == 0) { sscanf(line, "Price: %f",       &currentItem->price);}
    else if (strncmp(line, "Type:",  5) == 0) { sscanf(line, "Type: %19[^\n]",  currentItem->type); count++; }
  }

  fclose(file);
  return count;
}

// FIX #5: Room # is now parsed into reservation->roomNumber
int findBooking(const char *referenceNumber, Reservation *reservation) {
  FILE *file = fopen("bookings.txt", "r");
  if (!file) { printf("Could not open bookings.txt\n"); return 0; }

  char line[200];
  int  insideBlock = 0, found = 0;

  while (fgets(line, sizeof(line), file)) {
    line[strcspn(line, "\n")] = '\0';

    if (strncmp(line, "------- Guest Info", 18) == 0) {
      insideBlock = 1;
      memset(reservation, 0, sizeof(Reservation));
    }

    if (!insideBlock) continue;

    if      (strncmp(line, "Reference No:", 13) == 0) sscanf(line, "Reference No: %9s",      reservation->referenceNumber);
    else if (strncmp(line, "Room #:",        7) == 0) sscanf(line, "Room #: %d",              &reservation->roomNumber);   // FIX #5
    else if (strncmp(line, "Room Type:",    10) == 0) sscanf(line, "Room Type: %29[^\n]",     reservation->roomType);
    else if (strncmp(line, "Main Guest:",   11) == 0) sscanf(line, "Main Guest: %49[^\n]",    reservation->guestName);
    else if (strncmp(line, "No of Guest:",  12) == 0) sscanf(line, "No of Guest: %d",         &reservation->numberOfGuests);
    else if (strncmp(line, "Check-In:",      9) == 0) sscanf(line, "Check-In: %19[^\n]",      reservation->checkIn);
    else if (strncmp(line, "Checkout:",      9) == 0) sscanf(line, "Checkout: %19[^\n]",      reservation->checkOut);
    else if (strncmp(line, "No of Days:",   11) == 0) sscanf(line, "No of Days: %d",          &reservation->numberOfDays);
    else if (strncmp(line, "Room Rate:",    10) == 0) sscanf(line, "Room Rate: %f",            &reservation->roomRate);
    else if (strncmp(line, "Amenities Total:", 16) == 0) sscanf(line, "Amenities Total: %f",  &reservation->amenitiesTotal);
    else if (strncmp(line, "Final Amount:", 13) == 0) sscanf(line, "Final Amount: %f",         &reservation->finalAmount);
    else if (strncmp(line, "Status:",        7) == 0)
      reservation->isPaid = (strstr(line, "Not Paid") == NULL && strstr(line, "Paid") != NULL);

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

// FIX #6: Uses malloc instead of a 100KB stack array
void updateBookingPayment(Reservation *reservation) {
  FILE *file = fopen("bookings.txt", "r");
  if (!file) { printf("Could not open bookings.txt\n"); return; }

  char (*lines)[200] = malloc(MAX_BOOKINGS * sizeof(*lines));
  if (!lines) { printf("Memory error.\n"); fclose(file); return; }

  int totalLines = 0;
  while (totalLines < MAX_BOOKINGS && fgets(lines[totalLines], 200, file)) {
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

    if (inTargetBlock) {
      if      (strncmp(lines[i], "Amenities Total:", 16) == 0) sprintf(lines[i], "Amenities Total: %.2f",  reservation->amenitiesTotal);
      else if (strncmp(lines[i], "Final Amount:",    13) == 0) sprintf(lines[i], "Final Amount: %.2f",     reservation->finalAmount);
      else if (strncmp(lines[i], "Status:",           7) == 0) sprintf(lines[i], "Status: %s",             reservation->isPaid ? "Paid" : "Not Paid");
      else if (strncmp(lines[i], "Method:",           7) == 0) sprintf(lines[i], "Method: %s",             reservation->paymentMethod);
      else if (strncmp(lines[i], "Amount Received:", 16) == 0) sprintf(lines[i], "Amount Received: %.2f",  reservation->amountReceived);
      else if (strncmp(lines[i], "Change:",           7) == 0) sprintf(lines[i], "Change: %.2f",           reservation->change);
    }

    if (strncmp(lines[i], "===========", 11) == 0)
      inTargetBlock = 0;
  }

  file = fopen("bookings.txt", "w");
  if (file) {
    for (int i = 0; i < totalLines; i++)
      fprintf(file, "%s\n", lines[i]);
    fclose(file);
  } else {
    printf("Could not save payment.\n");
  }
  free(lines);
}

// NEW: Mark a room as Vacant in rooms.txt
void markRoomVacant(int roomNumber) {
  FILE *file = fopen("rooms.txt", "r");
  if (!file) { printf("Could not open rooms.txt\n"); return; }

  Room rooms[MAX_ROOMS];
  int  roomCount = 0;
  while (readRoom(file, &rooms[roomCount])) roomCount++;
  fclose(file);

  int found = 0;
  for (int i = 0; i < roomCount; i++) {
    if (rooms[i].roomNumber == roomNumber) {
      rooms[i].isAvailable = 1;
      found = 1;
      break;
    }
  }

  if (!found) {
    printf("Warning: Room #%03d not found in rooms.txt.\n", roomNumber);
    return;
  }

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
}

// NEW: Remove a booking block from bookings.txt
void removeBookingFromFile(const char *referenceNumber) {
  FILE *file = fopen("bookings.txt", "r");
  if (!file) { printf("Could not open bookings.txt\n"); return; }

  char (*lines)[200] = malloc(MAX_BOOKINGS * sizeof(*lines));
  if (!lines) { printf("Memory error.\n"); fclose(file); return; }

  int totalLines = 0;
  while (totalLines < MAX_BOOKINGS && fgets(lines[totalLines], 200, file)) {
    lines[totalLines][strcspn(lines[totalLines], "\n")] = '\0';
    totalLines++;
  }
  fclose(file);

  // Find blockStart and blockEnd for the target reference number
  int blockStart = -1, blockEnd = -1;

  for (int i = 0; i < totalLines; i++) {
    if (strncmp(lines[i], "------- Guest Info", 18) == 0) {
      // Look ahead for the reference number
      for (int j = i + 1; j < totalLines; j++) {
        if (strncmp(lines[j], "===========", 11) == 0) break;
        if (strncmp(lines[j], "Reference No:", 13) == 0) {
          char tmp[10] = "";
          sscanf(lines[j], "Reference No: %9s", tmp);
          if (strcmp(tmp, referenceNumber) == 0) {
            // Include the blank line that precedes this block
            blockStart = (i > 0 && strlen(lines[i - 1]) == 0) ? i - 1 : i;
          }
          break;
        }
      }
    }
    if (blockStart != -1 && strncmp(lines[i], "===========", 11) == 0) {
      blockEnd = i;
      break;
    }
  }

  if (blockStart == -1 || blockEnd == -1) {
    printf("Could not locate booking block in file.\n");
    free(lines);
    return;
  }

  file = fopen("bookings.txt", "w");
  if (!file) { printf("Could not rewrite bookings.txt\n"); free(lines); return; }

  for (int i = 0; i < totalLines; i++) {
    if (i >= blockStart && i <= blockEnd) continue; // skip the deleted block
    fprintf(file, "%s\n", lines[i]);
  }
  fclose(file);
  free(lines);
}

// NEW: Generate a receipt file after payment
void generateReceipt(Reservation *reservation) {
  char filename[30];
  sprintf(filename, "receipt_%s.txt", reservation->referenceNumber);

  FILE *file = fopen(filename, "w");
  if (!file) { printf("Could not create receipt file.\n"); return; }

  fprintf(file, "========================================\n");
  fprintf(file, "         ESPLENIN HOTEL RECEIPT\n");
  fprintf(file, "========================================\n");
  fprintf(file, "Reference No  : %s\n", reservation->referenceNumber);
  fprintf(file, "Guest Name    : %s\n", reservation->guestName);
  fprintf(file, "Room #        : %03d\n", reservation->roomNumber);
  fprintf(file, "Room Type     : %s\n", reservation->roomType);
  fprintf(file, "Check-In      : %s\n", reservation->checkIn);
  fprintf(file, "Check-Out     : %s\n", reservation->checkOut);
  fprintf(file, "No. of Guests : %d\n", reservation->numberOfGuests);
  fprintf(file, "No. of Days   : %d\n", reservation->numberOfDays);
  fprintf(file, "----------------------------------------\n");

  // Write amounts without printWithCommas (it uses printf, not fprintf)
  fprintf(file, "Room Rate     : PHP %.2f\n", reservation->roomRate);
  fprintf(file, "Amenities     : PHP %.2f\n", reservation->amenitiesTotal);
  fprintf(file, "----------------------------------------\n");
  fprintf(file, "TOTAL DUE     : PHP %.2f\n", reservation->finalAmount);
  fprintf(file, "Amount Paid   : PHP %.2f\n", reservation->amountReceived);
  fprintf(file, "Change        : PHP %.2f\n", reservation->change);
  fprintf(file, "Method        : %s\n",       reservation->paymentMethod);
  fprintf(file, "========================================\n");
  fprintf(file, "     Thank you for choosing us!\n");
  fprintf(file, "========================================\n");

  fclose(file);
  printf("Receipt saved to %s\n", filename);
}
