#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#define MAX 100

int choice;
FILE *fptr;

void displayMenu();
void chooseOption();
void listVacant();
void reserve();
void generateID(char *idOut);

typedef struct
{
    int roomNum;
    char category[30];
    int bedrooms;
    float price;
    int isAvailable;
    int nights;

    char guestName[50];
    char guestID[20];
    char guestPhone[15];
} Room;

int main()
{

    do
    {
        displayMenu();
        chooseOption();
    } while (choice != 7);

    return 0;
}

void displayMenu()
{
    printf("\n--- ESPLENIN HOTEL ---\n");
    printf("1. Vacancies\n");
    printf("2. Reserve\n");
    printf("3. Details\n");
    printf("4. Registry\n");
    printf("5. Checkout\n");
    printf("6. Inquiry\n");
    printf("7. Exit\n");
}

void chooseOption()
{
    printf("Selection: ");
    if (scanf("%d", &choice) == 0)
    {
        while (getchar() != '\n')
            ;
        printf("Invalid input. Please enter a number.\n");
        return;
    };
    while (getchar() != '\n')
        ;

    switch (choice)
    {
    case 1:
        listVacant();
        break;
    case 2:
        reserve();
        break;
    case 3:
        printf("Details: coming soon.\n");
        break;
    case 4:
        printf("Registry: coming soon.\n");
        break;
    case 5:
        printf("Checkout: coming soon.\n");
        break;
    case 6:
        printf("Inquiry: coming soon.\n");
        break;
    case 7:
        printf("Exiting ESPLENIN HOTEL system...\n");
        break;
    default:
        printf("Invalid choice. Try again.\n");
    }
}

int readRoom(FILE *fptr, Room *r)
{
    char line[150];
    char statusBuf[30];
    int filled = 0;

    while (fgets(line, sizeof(line), fptr))
    {
        line[strcspn(line, "\n")] = '\0';

        if (strncmp(line, "Room #", 6) == 0)
        {
            sscanf(line, "Room #%d:", &r->roomNum);
            filled++;
        }
        else if (strncmp(line, "Category:", 9) == 0)
        {
            sscanf(line, "Category: %29[^\n]", r->category);
            filled++;
        }
        else if (strncmp(line, "Bedrooms:", 9) == 0)
        {
            sscanf(line, "Bedrooms: %d", &r->bedrooms);
            filled++;
        }
        else if (strncmp(line, "Price:", 6) == 0)
        {
            sscanf(line, "Price: %f", &r->price);
            filled++;
        }
        else if (strncmp(line, "Status:", 7) == 0)
        {
            sscanf(line, "Status: %29[^\n]", statusBuf);
            r->isAvailable = (strstr(line, "Vacant") != NULL);
            filled++;
        }
        else if (strncmp(line, "Guest:", 6) == 0)
        {
            sscanf(line, "Guest: %49[^\n]", r->guestName);
            filled++;
        }
        else if (strncmp(line, "ID:", 3) == 0)
        {
            sscanf(line, "ID: %19[^\n]", r->guestID);
            filled++;
        }
        else if (strncmp(line, "Phone:", 6) == 0)
        {
            sscanf(line, "Phone: %14[^\n]", r->guestPhone);
            filled++;
        }
        else if (strncmp(line, "Nights:", 7) == 0)
        {
            sscanf(line, "Nights: %d", &r->nights);
            filled++;
        }

        // blank line = end of this room block
        if (line[0] == '\0' && filled > 0)
            break;
    }

    return filled > 0;
}

void listVacant() {
    fptr = fopen("rooms.txt", "r");
    if (fptr == NULL)
    {
        printf("Could not open rooms.txt\n");
        return;
    }

    Room room;
    int found = 0;

    printf("\n--- VACANT ROOMS ---\n");

    while (readRoom(fptr, &room))
    {
        if (room.isAvailable)
        {
            printf("Room #%03d | %d bed(s) | PHP %.2f/night\n",
                    room.roomNum, room.bedrooms, room.price);
            found = 1;
        }
    }

    if (!found)
        printf("No vacant rooms currently.\n");
    fclose(fptr);
}


// Option 2
void reserve() {
    fptr = fopen("rooms.txt", "r");
    if (fptr == NULL) { printf("Could not open rooms.txt\n"); return; }

    Room rooms[MAX];
    int count = 0;

    while (readRoom(fptr, &rooms[count]))
        count++;

    fclose(fptr);

    // show only vacant rooms
    printf("\n--- AVAILABLE ROOMS ---\n");
    int anyVacant = 0;
    for (int i = 0; i < count; i++) {
        if (rooms[i].isAvailable) {
            printf("Room #%03d | %d bed(s) | PHP %.2f/night\n",
                   rooms[i].roomNum, rooms[i].bedrooms, rooms[i].price);
            anyVacant = 1;
        }
    }
    if (!anyVacant) { printf("No vacant rooms.\n"); return; }

    // pick a room
    int pick;
    printf("\nEnter room number to reserve: ");
    scanf("%d", &pick);
    while (getchar() != '\n');

    // Find room chosen by user 
    int idx = -1;
    for (int i = 0; i < count; i++) {
        if (rooms[i].roomNum == pick && rooms[i].isAvailable) {
            idx = i;
            break;
        }
    }
    if (idx == -1) { printf("Room not found or already occupied.\n"); return; }

    printf("Guest Name  : ");
    fgets(rooms[idx].guestName, sizeof(rooms[idx].guestName), stdin);
    rooms[idx].guestName[strcspn(rooms[idx].guestName, "\n")] = '\0';

    printf("Phone Number: ");
    fgets(rooms[idx].guestPhone, sizeof(rooms[idx].guestPhone), stdin);
    rooms[idx].guestPhone[strcspn(rooms[idx].guestPhone, "\n")] = '\0';

    printf("Nights      : ");
    scanf("%d", &rooms[idx].nights);
    while (getchar() != '\n');

    rooms[idx].isAvailable = 0;
    
    generateID(rooms[idx].guestID);

    float total = rooms[idx].price * rooms[idx].nights;
    printf("\n--- RESERVATION SUMMARY ---\n");
    printf("Room    : #%03d %s\n", rooms[idx].roomNum, rooms[idx].category);
    printf("Guest   : %s\n", rooms[idx].guestName);
    printf("Guest ID: %s\n", rooms[idx].guestID);
    printf("Phone   : %s\n", rooms[idx].guestPhone);
    printf("Nights  : %d\n", rooms[idx].nights);
    printf("Total   : PHP %.2f\n", total);

    // Put it in rooms.txt file
    fptr = fopen("rooms.txt", "w");
    if (fptr == NULL) { printf("Could not save reservation.\n"); return; }

    for (int i = 0; i < count; i++) {
        fprintf(fptr, "Room #%03d:\n",        rooms[i].roomNum);
        fprintf(fptr, "Category: %s\n",       rooms[i].category);
        fprintf(fptr, "Bedrooms: %d\n",       rooms[i].bedrooms);
        fprintf(fptr, "Price: %.2f\n",        rooms[i].price);
        fprintf(fptr, "Status: %s\n",         rooms[i].isAvailable ? "Vacant" : "Occupied");
        fprintf(fptr, "Guest: %s\n",          rooms[i].guestName);
        fprintf(fptr, "ID: %s\n",             rooms[i].guestID);
        fprintf(fptr, "Phone: %s\n",          rooms[i].guestPhone);
        fprintf(fptr, "Nights: %d\n",         rooms[i].nights);
        fprintf(fptr, "\n");
    }

    fclose(fptr);
    printf("\nRoom #%03d successfully reserved!\n", rooms[idx].roomNum);
}

void generateID(char *idOut) {
    static int seeded = 0;
    if (!seeded) { srand(time(NULL)); seeded = 1; }

    int id = 100000000 + rand() % 900000000;
    sprintf(idOut, "%d", id);
}


void findGuest(){

}

void payment(){

}