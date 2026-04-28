#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 100

int choice;
FILE *fptr;

void displayMenu();
void chooseOption();
void listVacant();

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
    printf("\n--- GILDED LUXURY ---\n");
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
        printf("Reserve: coming soon.\n");
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
        printf("Exiting GILDED system...\n");
        break;
    default:
        printf("Invalid choice. Try again.\n");
    }
}

int readRoom(FILE *fptr, Room *r)
{
    char line[150];
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
            sscanf(line, "Status: %29[^\n]", r->category); // temp reuse
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

    return filled > 0; // returns 0 when no more rooms (EOF)
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
            printf("Room #%03d | %-20s | %d bed(s) | PHP %.2f/night\n",
                    room.roomNum, room.category, room.bedrooms, room.price);
            found = 1;
        }
    }

    if (!found)
        printf("No vacant rooms currently.\n");
    fclose(fptr);
}

void reserveRoom(){

}

void findGuest(){

}

void payment(){

}