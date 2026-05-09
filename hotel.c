#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#define MAX 100

int choice;

// ─── Structs ───────────────────────────────────────────────────────────────────

typedef struct {
  char refNo[10];
  char name[50];
  char checkIn[20];
  char checkOut[20];
  int guests;
  int days;
  char roomType[30];
  int roomNum;
  float pricePerNight;
  float roomRate;
  float amenitiesTotal;
  float finalAmount;
  float amountReceived;
  float change;
  char payMethod[20];
  int isPaid;
} Reservation;

typedef struct {
  int roomNum;
  char category[30];
  int bedrooms;
  float price;
  int isAvailable;
  char guestName[50];
  char guestID[20];
  char guestPhone[15];
  int days;
  char checkIn[20];
  char checkOut[20];
} Room;

// ─── Forward Declarations ──────────────────────────────────────────────────────

void displayMenu();
void chooseOption();
void listVacant();
void reserve();
void generateRefNo(char *refOut);
void viewDetails();
void registry();
void payment(const char *refNo, int fromReserve);
void updateBookingPayment(Reservation *res);
int findBooking(const char *refNo, Reservation *res);

// ─── Main ──────────────────────────────────────────────────────────────────────

int main() {
  do {
    displayMenu();
    chooseOption();
  } while (choice != 7);
  return 0;
}

// ─── Menu ──────────────────────────────────────────────────────────────────────

void displayMenu() {
  printf("\n--- ESPLENIN HOTEL ---\n");
  printf("1. Vacancies\n");
  printf("2. Reserve\n");
  printf("3. Payment\n");
  printf("4. Registry\n");
  printf("5. Details\n");
  printf("6. Checkout\n");
  printf("7. Exit\n");
}

void chooseOption() {
  printf("Selection: ");
  if (scanf("%d", &choice) == 0) {
    while (getchar() != '\n')
      ;
    printf("Invalid input. Please enter a number.\n");
    return;
  }

  while (getchar() != '\n');

  switch (choice) {
  case 1:
    listVacant();
    break;
  case 2:
    reserve();
    break;
  case 3: {
    char ref[10];
    printf("Enter Reference No: ");
    scanf("%9s", ref);
    while (getchar() != '\n')
      ;
    payment(ref, 0);
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
}

// ─── File Reader ───────────────────────────────────────────────────────────────

int readRoom(FILE *fp, Room *r) {
  char line[150];
  int filled = 0;
  memset(r, 0, sizeof(Room));

  while (fgets(line, sizeof(line), fp)){
    line[strcspn(line, "\n")] = '\0';

    if (strncmp(line, "Room #", 6) == 0) {
      sscanf(line, "Room #%d:", &r->roomNum);
      filled++;
    }
    else if (strncmp(line, "Category:", 9) == 0) {
      sscanf(line, "Category: %29[^\n]", r->category);
      filled++;
    }
    else if (strncmp(line, "Bedrooms:", 9) == 0) {
      sscanf(line, "Bedrooms: %d", &r->bedrooms);
      filled++;
    }
    else if (strncmp(line, "Price:", 6) == 0) {
      sscanf(line, "Price: %f", &r->price);
      filled++;
    }
    else if (strncmp(line, "Status:", 7) == 0) {
      r->isAvailable = (strstr(line, "Vacant") != NULL);
      filled++;
    }

    if (line[0] == '\0' && filled > 0)
      break;
  }

  return filled > 0;
}

// ─── Option 1: List Vacant ─────────────────────────────────────────────────────

void listVacant() {
  FILE *fptr = fopen("rooms.txt", "r");
  if (!fptr) {
    printf("Could not open rooms.txt\n");
    return;
  }

  Room room;
  int found = 0;

  printf("\n--- VACANT ROOMS ---\n");
  while (readRoom(fptr, &room)) {
    if (room.isAvailable) {
      printf("Room #%03d | %-20s | %d bed(s) | PHP %.2f/night\n",
             room.roomNum, room.category, room.bedrooms, room.price);
      found = 1;
    }
  }
  if (!found)
    printf("No vacant rooms currently.\n");
  fclose(fptr);
}

// ─── Option 2: Reserve ────────────────────────────────────────────────────────

void generateRefNo(char *refOut) {
  int count = 0;
  FILE *fp = fopen("bookings.txt", "r");
  if (fp) {
    char line[200];
    while (fgets(line, sizeof(line), fp))
      if (strncmp(line, "Reference No:", 13) == 0)
        count++;
    fclose(fp);
  }
  sprintf(refOut, "B%04d", count + 1);
}

void reserve() {
  char payNow;
  
  FILE *fptr = fopen("rooms.txt", "r");
  if (!fptr) {
    printf("Could not open rooms.txt\n");
    return;
  }

  Room rooms[MAX];
  int count = 0;
  while (readRoom(fptr, &rooms[count]))
    count++;
  fclose(fptr);

  // show available rooms
  printf("\n--- AVAILABLE ROOMS ---\n");
  int anyVacant = 0;
  for (int i = 0; i < count; i++) {
    if (rooms[i].isAvailable)
    {
      printf("Room #%03d | %-20s | %d bed(s) | PHP %.2f/night\n",
             rooms[i].roomNum, rooms[i].category,
             rooms[i].bedrooms, rooms[i].price);
      anyVacant = 1;
    }
  }
  if (!anyVacant) {
    printf("No vacant rooms.\n");
    return;
  }

  // gather guest info
  char checkIn[20], checkOut[20], guestName[50];
  int numGuests, numDays;

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
  scanf("%d", &numGuests);
  while (getchar() != '\n')
    ;

  printf("No. of Days  : ");
  scanf("%d", &numDays);
  while (getchar() != '\n')
    ;

  // room type selection
  printf("\nRoom Type:\n");
  printf("  [A] - De Luxe\n");
  printf("  [B] - Suite\n");
  printf("  [C] - Luxury Suite\n");
  printf("Choice: ");
  char typePick;
  scanf("%c", &typePick);
  while (getchar() != '\n');

  typePick = toupper(typePick);

  const char *chosenCategory;
  switch (typePick) {
    case 'A':
      chosenCategory = "De Luxe";
      break;
    case 'B':
      chosenCategory = "Suite";
      break;
    case 'C':
      chosenCategory = "Luxury Suite";
      break;
    default:
      printf("Invalid room type choice.\n");
      return;
  }

  // find first vacant room of chosen type
  int idx = -1;
  for (int i = 0; i < count; i++) {
    if (rooms[i].isAvailable &&
        strcmp(rooms[i].category, chosenCategory) == 0)
    {
      idx = i;
      break;
    }
  }

  if (idx == -1) {
    printf("Sorry, no vacant %s rooms available.\n", chosenCategory);
    return;
  }

  strcpy(rooms[idx].checkIn, checkIn);
  strcpy(rooms[idx].checkOut, checkOut);
  strcpy(rooms[idx].guestName, guestName);
  rooms[idx].days = numDays;

  float roomRate = rooms[idx].price * numDays;

  char refNo[10];
  generateRefNo(refNo);

  // summary
  printf("\n ----------  RESERVATION SUMMARY  ---------- \n");
  printf("Room Type     : %s\n", chosenCategory);
  printf("Price/night   : PHP %.2f\n", rooms[idx].price);
  printf("Room Rate     : PHP %.2f (PHP %.2f x %d days)\n",
    roomRate, rooms[idx].price, numDays);
  printf("Reference No  : %s\n", refNo);

  // mark room occupied in rooms.txt
  rooms[idx].isAvailable = 0;
  fptr = fopen("rooms.txt", "w");
  if (!fptr) {
    printf("Could not update rooms.txt\n");
    return;
  }

  for (int i = 0; i < count; i++) {
    fprintf(fptr, "Room #%03d:\n", rooms[i].roomNum);
    fprintf(fptr, "Category: %s\n", rooms[i].category);
    fprintf(fptr, "Bedrooms: %d\n", rooms[i].bedrooms);
    fprintf(fptr, "Price: %.2f\n", rooms[i].price);
    fprintf(fptr, "Status: %s\n", rooms[i].isAvailable ? "Vacant" : "Occupied");
    fprintf(fptr, "\n");
  }
  fclose(fptr);

  // append to bookings.txt
  fptr = fopen("bookings.txt", "a");
  if (!fptr) {
    printf("Could not open bookings.txt\n");
    return;
  }

  // Write in bookings.txt
  fprintf(fptr, "\n");
  fprintf(fptr, "------- Guest Info -------\n");
  fprintf(fptr, "Reference No: %s\n", refNo);
  fprintf(fptr, "Room #: %03d\n", rooms[idx].roomNum);
  fprintf(fptr, "Room Type: %s\n", chosenCategory);
  fprintf(fptr, "Main Guest: %s\n", guestName);
  fprintf(fptr, "No of Guest: %d\n", numGuests);
  fprintf(fptr, "Check-In: %s\n", checkIn);
  fprintf(fptr, "Checkout: %s\n", checkOut);
  fprintf(fptr, "No of Days: %d\n", numDays);
  fprintf(fptr, "\n");
  fprintf(fptr, "------- Bill Info -------\n");
  fprintf(fptr, "Room Rate: %.2f\n", roomRate);
  fprintf(fptr, "Amenities Used:\n");
  fprintf(fptr, "Amenities Total: 0.00\n");
  fprintf(fptr, "Final Amount: %.2f\n", roomRate);
  fprintf(fptr, "\n");
  fprintf(fptr, "------- Payment Info -------\n");
  fprintf(fptr, "Status: Not Paid\n");
  fprintf(fptr, "Method:\n");
  fprintf(fptr, "Amount Received:\n");
  fprintf(fptr, "Change:\n");
  fprintf(fptr, "\n");
  fprintf(fptr, "===========================================================\n");
  fclose(fptr);

  // Display ———————————————————————————————————————————————————
  printf("Booking\n");
  printf("Input Reference No. : %s\n", refNo);
  printf("Details:\n");
  printf("   Check In: %s\n", checkIn);
  printf("   Check Out: %s\n", checkOut);
  printf("   Name of Main Guest: %s\n", guestName);
  printf("   No. Of Guest: %d\n", numGuests);
  printf("   No. Of days: %d\n", numDays);
  printf("   Room Type: %s\n", chosenCategory);
  printf("   Price/night: %.2f\n", );
  printf("   Room Rate: %.2f\n", roomRate);


  // Offer payment now
  printf("\nProceed to payment now? [y/n]: ");
  scanf("%c", &payNow);
  while (getchar() != '\n');

  if (tolower(payNow) == 'y')
    payment(refNo, 1);
  else
    printf("Thank you for checking in! Enjoy your room!\n");
}

// ─── Option 3: Payment ────────────────────────────────────────────────────────

int findBooking(const char *refNo, Reservation *res) {
  FILE *fp = fopen("bookings.txt", "r");
  if (!fp) {
    printf("Could not open bookings.txt\n");
    return 0;
  }

  char line[200];
  int inBlock = 0, filled = 0;

  while (fgets(line, sizeof(line), fp)) {
    line[strcspn(line, "\n")] = '\0';

    if (strncmp(line, "------- Guest Info", 18) == 0){
      inBlock = 1;
      memset(res, 0, sizeof(Reservation));
    }

    if (!inBlock)
      continue;

    if (strncmp(line, "Reference No:", 13) == 0)
      sscanf(line, "Reference No: %9s", res->refNo);
    else if (strncmp(line, "Room Type:", 10) == 0)
      sscanf(line, "Room Type: %29[^\n]", res->roomType);
    else if (strncmp(line, "Main Guest:", 11) == 0)
      sscanf(line, "Main Guest: %49[^\n]", res->name);
    else if (strncmp(line, "No of Guest:", 12) == 0)
      sscanf(line, "No of Guest: %d", &res->guests);
    else if (strncmp(line, "Check-In:", 9) == 0)
      sscanf(line, "Check-In: %19[^\n]", res->checkIn);
    else if (strncmp(line, "Checkout:", 9) == 0)
      sscanf(line, "Checkout: %19[^\n]", res->checkOut);
    else if (strncmp(line, "No of Days:", 11) == 0)
      sscanf(line, "No of Days: %d", &res->days);
    else if (strncmp(line, "Room Rate:", 10) == 0)
      sscanf(line, "Room Rate: %f", &res->roomRate);
    else if (strncmp(line, "Amenities Total:", 16) == 0)
      sscanf(line, "Amenities Total: %f", &res->amenitiesTotal);
    else if (strncmp(line, "Final Amount:", 13) == 0)
      sscanf(line, "Final Amount: %f", &res->finalAmount);
    else if (strncmp(line, "Status:", 7) == 0)
      res->isPaid = (strstr(line, "Not Paid") == NULL && strstr(line, "Paid") != NULL);

    if (strncmp(line, "===========", 11) == 0 && inBlock) {
      inBlock = 0;
      if (strcmp(res->refNo, refNo) == 0) {
        filled = 1;
        break;
      }
    }
  }

  fclose(fp);
  return filled;
}

void updateBookingPayment(Reservation *res) {
  FILE *fp = fopen("bookings.txt", "r");
  if (!fp) {
    printf("Could not open bookings.txt\n");
    return;
  }

  char lines[500][200];
  int total = 0;
  while (total < 500 && fgets(lines[total], sizeof(lines[total]), fp)) {
    lines[total][strcspn(lines[total], "\n")] = '\0';
    total++;
  }
  fclose(fp);

  int inTarget = 0;
  for (int i = 0; i < total; i++) {
    if (strncmp(lines[i], "Reference No:", 13) == 0) {
      char tmp[10];
      sscanf(lines[i], "Reference No: %9s", tmp);
      inTarget = (strcmp(tmp, res->refNo) == 0);
    }

    if (!inTarget)
      continue;

    if (strncmp(lines[i], "Amenities Total:", 16) == 0)
      sprintf(lines[i], "Amenities Total: %.2f", res->amenitiesTotal);
    else if (strncmp(lines[i], "Final Amount:", 13) == 0)
      sprintf(lines[i], "Final Amount: %.2f", res->finalAmount);
    else if (strncmp(lines[i], "Status:", 7) == 0)
      sprintf(lines[i], "Status: %s", res->isPaid ? "Paid" : "Not Paid");
    else if (strncmp(lines[i], "Method:", 7) == 0)
      sprintf(lines[i], "Method: %s", res->payMethod);
    else if (strncmp(lines[i], "Amount Received:", 16) == 0)
      sprintf(lines[i], "Amount Received: %.2f", res->amountReceived);
    else if (strncmp(lines[i], "Change:", 7) == 0)
      sprintf(lines[i], "Change: %.2f", res->change);

    if (strncmp(lines[i], "===========", 11) == 0)
      inTarget = 0;
  }

  fp = fopen("bookings.txt", "w");
  if (!fp)
  {
    printf("Could not save payment.\n");
    return;
  }
  for (int i = 0; i < total; i++)
    fprintf(fp, "%s\n", lines[i]);
  fclose(fp);
}

void payment(const char *refNo, int fromReserve){
  Reservation res;

  if (!findBooking(refNo, &res)){
    printf("Booking %s not found.\n", refNo);
    return;
  }

  if (res.isPaid){
    printf("Booking %s is already paid.\n", refNo);
    return;
  }

  // ask amenities only when coming straight from reserve()
  if (fromReserve){
    printf("\n--- AMENITIES ---\n");
    printf("\nRoom Type:\n");
    printf("  [A] - De Luxe\n");
    printf("  [B] - Suite\n");
    printf("  [C] - Luxury Suite\n");
    printf("Choice: ");
    char typePick;
    scanf("%c", &typePick);
    while (getchar() != '\n');
    
    // this part will print the sesction

  }

  res.finalAmount = res.roomRate + res.amenitiesTotal;

  // Bill Breakdown
  printf("\n========================================\n");
  printf("            BILL BREAKDOWN\n");
  printf("========================================\n");
  printf("Reference No  : %s\n", res.refNo);
  printf("Main Guest    : %s\n", res.name);
  printf("Check-In      : %s\n", res.checkIn);
  printf("Check-Out     : %s\n", res.checkOut);
  printf("No. of Days   : %d\n", res.days);
  printf("Room Type     : %s\n", res.roomType);
  printf("----------------------------------------\n");
  printf("Room Rate     : PHP %.2f\n", res.roomRate);
  printf("Amenities     : PHP %.2f\n", res.amenitiesTotal);
  printf("----------------------------------------\n");
  printf("TOTAL DUE     : PHP %.2f\n", res.finalAmount);
  printf("========================================\n");

  printf("\nDo you want to proceed with payment? [y/n]: ");
  char confirm;
  scanf("%c", &confirm);
  while (getchar() != '\n');

  if (tolower(confirm) != 'y'){
    printf("Payment cancelled. Returning to main menu.\n");
    return;
  }  

  printf("Amount Received: PHP ");
  scanf("%f", &res.amountReceived);
  while (getchar() != '\n');

  if (res.amountReceived < res.finalAmount){
    printf("Insufficient amount. Payment not processed.\n");
    return;
  }

  res.change = res.amountReceived - res.finalAmount;
  res.isPaid = 1;
  strcpy(res.payMethod, "Cash");

  printf("\nChange        : PHP %.2f\n", res.change);
  printf("Payment successful! Thank you, %s.\n", res.name);

  updateBookingPayment(&res);
}








// ─── Option 4: Registry ───────────────────────────────────────────────────────

void registry() {
  char searchName[50];
  char again;

  do {
    printf("\n--- REGISTRY ---\n");
    printf("Enter first name to search: ");
    fgets(searchName, sizeof(searchName), stdin);
    searchName[strcspn(searchName, "\n")] = '\0';

    FILE *fp = fopen("bookings.txt", "r");
    if (!fp) {
      printf("Could not open bookings.txt\n");
      return;
    }

    char line[200];
    char blockLines[20][200];
    int blockCount = 0;
    int inBlock = 0;
    int anyFound = 0;

    while (fgets(line, sizeof(line), fp)) {
      line[strcspn(line, "\n")] = '\0';

      if (strncmp(line, "------- Guest Info", 18) == 0) {
        inBlock = 1;
        blockCount = 0;
        memset(blockLines, 0, sizeof(blockLines));
      }

      if (inBlock && blockCount < 20)
        strcpy(blockLines[blockCount++], line);

      if (strncmp(line, "Main Guest:", 11) == 0) {
        char fullName[50] = "", firstName[50] = "";
        sscanf(line, "Main Guest: %49[^\n]", fullName);
        sscanf(fullName, "%49s", firstName);

        if (strcasecmp(firstName, searchName) == 0)
        {
          anyFound = 1;
          printf("\n--- BOOKING FOUND ---\n");
          for (int i = 0; i < blockCount; i++)
            printf("%s\n", blockLines[i]);
        }
      }

      if (strncmp(line, "===========", 11) == 0 && inBlock) {
        if (anyFound)
          printf("%s\n", line);
        inBlock = 0;
        blockCount = 0;
      }
    }

    fclose(fp);
    if (!anyFound)
      printf("No bookings found for \"%s\".\n", searchName);

    printf("\nSearch another guest? [y/n]: ");
    scanf("%c", &again);
    while (getchar() != '\n');

  } while (tolower(again) == 'y');

  printf("Returning to main menu.\n");
}

// ─── Option 5: View Room Details ──────────────────────────────────────────────

void viewDetails() {
  FILE *fp = fopen("rooms.txt", "r");
  if (!fp) {
    printf("Could not open rooms.txt\n");
    return;
  }

  Room rooms[MAX];
  int count = 0;
  while (readRoom(fp, &rooms[count]))
    count++;
  fclose(fp);

  printf("\n--- ALL ROOMS ---\n");
  for (int i = 0; i < count; i++) {
    printf("Room #%03d | %-20s | %s\n",
           rooms[i].roomNum, rooms[i].category,
           rooms[i].isAvailable ? "Vacant" : "Occupied");
  }

  int pick;
  printf("\nEnter room number to view: ");
  scanf("%d", &pick);
  while (getchar() != '\n');

  int idx = -1;
  for (int i = 0; i < count; i++) {
    if (rooms[i].roomNum == pick) {
      idx = i;
      break;
    }
  }

  if (idx == -1) {
    printf("Room #%03d not found.\n", pick);
    return;
  }

  Room r = rooms[idx];
  printf("\n--- ROOM DETAILS ---\n");
  printf("Room     : #%03d\n", r.roomNum);
  printf("Category : %s\n", r.category);
  printf("Bedrooms : %d\n", r.bedrooms);
  printf("Price    : PHP %.2f/night\n", r.price);
  printf("Status   : %s\n", r.isAvailable ? "Vacant" : "Occupied");
}