#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// ---------------------------------------------------------------------
// Screen initialaziton. width = 80 in V4
// ---------------------------------------------------------------------
int high  = 20;
int width = 60;



int maxEnemys = 30;
int maxItems  = 30;



int userNameLen = 50;
int passLen     = 50;
int emailLen    = 100;


// ---------------------------------------------------------------------
// used U-2863 in previos
// ---------------------------------------------------------------------

char flooor   = '.';
char gamer   = '@';
char wall1   = '|';
char wall2   = '_';
char RahRo   = '#';
char pillar  = 'O';
char windows = '=';

// ---------------------------------------------------------------------
// defining The Colors
// full instruction for use which color for what type use رنگ بندی ها.txt
// ---------------------------------------------------------------------
#define Cwall    1
#define Cfloor   2
#define Cgamer   3
#define CEnemy   4
#define Cgold    5
#define Ctrap    6
#define Cfood    7
#define C2Items  8

// ---------------------------------------------------------------------
// User Password initilizition
// ---------------------------------------------------------------------
// این ساختار برای نگهداری اطلاعات کاربر در فایل users.dat استفاده می‌شود


typedef struct {
    char username[50];
    char password[50];
    char email[100];
} User;

// ---------------------------------------------------------------------
// Guest should have no id.changed must not be saved...
// ---------------------------------------------------------------------
User currentUser;


typedef struct {
    char symbol;       // نماد سلول (مثلاً دیوار، کف، ...)    //نماد کاراکتر اون سلول رو با این تعریف کردی.
    int visible;       
    int blocked;       
    int enemyIndex;    // اگر >=0 باشد، نشان‌دهنده حضور یک دشمن با این ایندکس
    int iii;           
} Cell;

/*ارایه سراسری کل نقشه اینه */
Cell gameMap[20][60];  // همان high=20, width=60

// ---------------------------------------------------------------------
// use for random Room making(در تابع gen rooms ورژن دو تغیرش دادم)
// ---------------------------------------------------------------------
typedef struct {
    int x, y;  // مختصات بالای چپ اتاق
    int w, h;  // عرض و ارتفاع اتاق
} Room;

// ---------------------------------------------------------------------
// Monsters and enemies Type!!!!
// ---------------------------------------------------------------------
typedef enum {
    eneDaemon,
    eneFire,
    eneGiant,
    eneSnake,
    eneUndead
} EnemyType;

// ---------------------------------------------------------------------
// Init Them...
// ---------------------------------------------------------------------
typedef struct {
    int active;     // 0=غیرفعال، 1=فعال
    EnemyType type;
    int x, y;      
    int health;     
    int damage;    
    int canMove;    //added this due to recomendition...
    //might Be deleted or changed!
} Enemy;

/*  در اینجا تغییر میکنه آرایه سراسری همه دشمنان بازی */
Enemy enemies[30];// ماکسیمم سی تا دشمن


// ---------------------------------------------------------------------
// Food Type init
// dont used yet....
// ---------------------------------------------------------------------

typedef enum {
    normal,
    advanced,
    magic,
    rotten
} FoodType;

/* اسم weapon ها : WEAPON_MACE -> mace, ... (کوچک) */
typedef enum {
    mace,
    dagger,
    wand,
    arrow,
    sword
} WeaponType;

/* ساختار Weapon؛ quantity -> NumberOf */
typedef struct {
    WeaponType wtype;
    int NumberOf;
} Weapon;

/* آیتم‌ها */
typedef enum {
    ITEM_GOLD,
    ITEM_TRAP,
    ITEM_FOOD,
    ITEM_WEAPON,
    ITEM_SPELL
} ItemType;

/* 

  spellType= damage
    player.damage قاطی نشه.
   
*/
typedef enum {
    health,
    speed,
    damage
} SpellType;

/* ساختار آیتم*/
typedef struct {
    int active;
    ItemType type;
    int x, y;
    union {
        FoodType   foodType;
        WeaponType wtype;
        SpellType  spellType;
        int goldAmount;
    } subtype;
    int NumberOf; // در تعریف برای اسپل شاید متغیر جدید تعریف شه.
} Item;


Item items[30]; // maxItems=30

// ---------------------------------------------------------------------
// i define Player here...
// max could be chnaged...
// ---------------------------------------------------------------------


typedef struct {
    int x, y;  
    int health;
    int maxHealth;
    int gold;
    int score;
    int speed;
    int damage;  //قاطی نشه...

    Weapon currentWeapon;
    Weapon inventory[10];
    int weaponCount;
} Player;


Player player;

// ---------------------------------------------------------------------
// Weapon damage changed ...
// Not working on V3...
// dont used yet....
// ---------------------------------------------------------------------

int getWeaponDamage(WeaponType wt) {
    switch(wt) {
        case mace:
           return 5;   
        case dagger:
         return 12;
        case wand:
           return 15;
        case arrow:
          return 5;  
        case sword:
          return 10; 
    }
    return 0;
}

int getWeaponRange(WeaponType wt) {
    switch(wt) {
        case dagger: return 5;
        case wand:   return 15;
        case arrow:  return 5;
        case mace:   return 1; 
        case sword:  return 1; 
    }
    return 0;
}

// این تابع رو برای چک کردن نزدیک برد یا دور برد بودن یک سلاح 
// قبلا در قالب  NearChecker استفاده کردی
// last changed worked.....

// please dont change this anymore !!!

int Far_check(WeaponType wt) {
    if(wt == dagger || wt == wand || wt == arrow) {
        return 1;
    }
    return 0;
}

/*  توابع مربوط به فایل کپی شده برای کار با فایل هاusers.dat */
/* این توابع اماده و کپی شده هستند*/
// تغییر نکند 
// last changed V3 on line 193

int loadUsers(User *users, int maxCount) {
    FILE *fp = fopen("users.dat", "rb");
    if(!fp) return 0;
    int cnt = fread(users, sizeof(User), maxCount, fp);
    fclose(fp);
    return cnt;
}

void saveUsers(User *users, int cnt) {
    FILE *fp = fopen("users.dat", "wb");
    if(!fp) return;
    fwrite(users, sizeof(User), cnt, fp);
    fclose(fp);
}
//

// ---------------------------------------------------------------------
// email validating
// do not use strcmp for domain part
// changed!
// ---------------------------------------------------------------------
int email_cheking(const char *email) {
    char *at = strchr(email, '@');
    if(!at) 
        return 0; // not valid..!
    char *dot = strchr(at, '.');
    if(!dot) 
        return 0; 
    return 1;
}

// ---------------------------------------------------------------------
// pass check (ctype not used in this only in V2)
// ---------------------------------------------------------------------

int password_checking(const char *password) {
    if(strlen(password) < 7) 
        return 0;
    int hasUpper=0, hasLower=0, hasDigit=0;
    for(int i=0; password[i]; i++){
        if(password[i]>='A' && password[i]<='Z') 
            hasUpper=1;
        if(password[i]>='a' && password[i]<='z') 
            hasLower=1;
        if(password[i]>='0' && password[i]<='9') 
            hasDigit=1;
    }
    if(hasUpper && hasLower && hasDigit) 
        return 1;
    return 0;
}

// biding function heres..

void Sign_up() {
    echo();
    User users[100];
    int numberofuser = loadUsers(users, 100);

    mvprintw(LINES-4, 0, "Enter your new username: ");
    char uname[50];
    getnstr(uname, 49);

// ---------------------------------------------------------------------
// might not be chnaged
// verifiying Entered Username
// ---------------------------------------------------------------------
    for(int i=0; i<numberofuser; i++){
        if(strcmp(uname, users[i].username)==0){
            mvprintw(LINES-3, 0, "This Username already exists.....Please change it...... or  press any key...");
            getch();
            noecho();
            return;
        }
    }

    mvprintw(LINES-3, 0, "enter new password: ");
    char pass[50];
    getnstr(pass, 49);
    if(!password_checking(pass)) {
        mvprintw(LINES-2, 0, "Invalid password....Please change it.... Press any key...");
        getch();
        noecho();
        return;
    }

    mvprintw(LINES-1, 0, "Enter email: ");
    char mail[100];
    getnstr(mail, 99);
    if(!email_cheking(mail)) {
        mvprintw(LINES-1, 0, "Invalid or Wrong email...Please change it or Press any key...");
        getch();
        noecho();
        return;
    }

// ---------------------------------------------------------------------
// new user generated...
// ---------------------------------------------------------------------
    User newU;
    strcpy(newU.username, uname);
    strcpy(newU.password, pass);
    strcpy(newU.email, mail);
    users[numberofuser++] = newU;
    saveUsers(users, numberofuser);

    mvprintw(LINES-1, 0, "User singed Up!!! Press any key...");
    getch();
    noecho();
}


int login() {
    echo();
    User users[100]; 
    int numberofuser = loadUsers(users, 100);

    mvprintw(LINES-4, 0, "Enter username: ");
    char uname[50];
    getnstr(uname, 49);

    mvprintw(LINES-3, 0, "Enter password: ");
    char pass[50];
    getnstr(pass, 49);

    noecho();

    for(int i=0; i<numberofuser; i++){
        if(strcmp(uname, users[i].username)==0 &&
           strcmp(pass, users[i].password)==0) {
            currentUser = users[i];
            mvprintw(LINES-2, 0, "Login successful...You LOGGED IN... Press any key...");
            getch();
            return 1;
        }
    }
    mvprintw(LINES-2, 0, "Login failed...Please change it... Press any key...");
    getch();
    return 0;
}


void generateMap() {
    

    for(int r=0; r<high; r++){
        for(int c=0; c<width; c++){
            gameMap[r][c].symbol = ' ';
            gameMap[r][c].blocked = 1;  
            gameMap[r][c].enemyIndex = -1;
            gameMap[r][c].iii  = -1; 
            gameMap[r][c].visible = 0; 
        }
    }

    int maxRooms = 10;
    Room rooms[10];
    int roomCount = 0;
//1 and 4 added to do high compict of rooms with edge of window...
//chnaged ..
    for(int i=0; i<maxRooms; i++){
        int w = 4 + rand()%7;
        int h = 4 + rand()%5;
        int x = 1 + rand() % (high - h - 2);
        int y = 1 + rand() % (width - w - 2);


        int TadaKhol=0;
        for(int j=0; j<roomCount; j++){
            if (x < rooms[j].x + rooms[j].h + 1 &&
                x + h + 1 > rooms[j].x &&
                y < rooms[j].y + rooms[j].w + 1 &&
                y + w + 1 > rooms[j].y) {
                TadaKhol=1;
                //regeneration theory might be changed from V4
                break;
            }
        }
        if(TadaKhol==1) 
            continue;
//crach case
//fixed 


        rooms[roomCount].x = x;
        rooms[roomCount].y = y;
        rooms[roomCount].w = w;
        rooms[roomCount].h = h;
        ++roomCount;

// ---------------------------------------------------------------------
// all of the map should be floor at first..
// Items added Next!!!
// dont change this part..
// ---------------------------------------------------------------------
        for(int rr=x; rr<x+h; rr++){
            for(int cc=y; cc<y+w; cc++){
                gameMap[rr][cc].symbol = flooor;
                gameMap[rr][cc].blocked=0;
                gameMap[rr][cc].visible=1;
            }
        }

//cc started from -1 
// changed from V3
// .cc = 0 lead to crash(segment fault ERRR.........)
        for(int cc=y-1; cc<=y+w; cc++){
            if(x-1>=0){
                gameMap[x-1][cc].symbol = (x-1==0)? windows : wall2;
                gameMap[x-1][cc].blocked=1;
                gameMap[x-1][cc].visible=1;
            }
            if(x+h<high){
                gameMap[x+h][cc].symbol = (x+h==high-1)? windows : wall2;
                gameMap[x+h][cc].blocked=1;
                gameMap[x+h][cc].visible=1;
            }
        }
/* right and left walls /.*/
        for(int rr=x-1; rr<=x+h; rr++){
            if(y-1>=0){
                gameMap[rr][y-1].symbol = (y-1==0)? windows : wall1;
                //handle..!
                //int ipath = (cc + 1) * 40///hight of room
                //just for safe...
    
                gameMap[rr][y-1].blocked=1;
                gameMap[rr][y-1].visible=1;
            }
            if(y+w<width){
                gameMap[rr][y+w].symbol = (y+w==width-1)? windows : wall1;
                // | adding 
                gameMap[rr][y+w].blocked=1;
                // making blocked...
                //for first
                //changed in 893
                gameMap[rr][y+w].visible=1;
            }
        }

//الگوریتم ستون های تصادفی کمک گرفته شده
//alghorihtm in V3 dont work properly
// changed 
        if(w>=6 && h>=6 && (rand()%100<20)){
            int pillarX = x+1 + rand()%(h-2);
            int pillarY = y+1 + rand()%(w-2);
            gameMap[pillarX][pillarY].symbol = pillar;
            int path_of_my_file = 1; //در صورت بروز مشکل باز کردن فایل به 0 تبدیلش کن*/
            gameMap[pillarX][pillarY].blocked=1;
            gameMap[pillarX][pillarY].visible=1;
        }
    }//??

//room connecting to RahRo 
//nt my algorhitm

    for(int i=1; i<roomCount; i++){
        int pCenterX = rooms[i-1].x + rooms[i-1].h/2;
        int pCenterY = rooms[i-1].y + rooms[i-1].w/2;
        int cCenterX = rooms[i].x + rooms[i].h/2;
        int cCenterY = rooms[i].y + rooms[i].w/2;

        if(rand()%2){
            //verical then horizontal
            for(int c=(pCenterY < cCenterY ? pCenterY : cCenterY);
                c <= (pCenterY > cCenterY ? pCenterY : cCenterY); c++){
                gameMap[pCenterX][c].symbol = RahRo;
                gameMap[pCenterX][c].blocked=0;
                gameMap[pCenterX][c].visible=1;
            }
            for(int r=(pCenterX < cCenterX ? pCenterX : cCenterX);
                r <= (pCenterX > cCenterX ? pCenterX : cCenterX); r++){
                gameMap[r][cCenterY].symbol = RahRo;
                gameMap[r][cCenterY].blocked=0;
                gameMap[r][cCenterY].visible=1;
            }
        } else {
            //Baraksesh Dagigan!!!!!
            for(int r=(pCenterX < cCenterX ? pCenterX : cCenterX);
                r <= (pCenterX > cCenterX ? pCenterX : cCenterX); r++){
                gameMap[r][pCenterY].symbol = RahRo;
                gameMap[r][pCenterY].blocked=0;
                gameMap[r][pCenterY].visible=1;
            }
            for(int c=(pCenterY < cCenterY ? pCenterY : cCenterY);
                c <= (pCenterY > cCenterY ? pCenterY : cCenterY); c++){
                gameMap[cCenterX][c].symbol = RahRo;
                gameMap[cCenterX][c].blocked=0;
                gameMap[cCenterX][c].visible=1;
            }
        }
    }

//puting walls ...
// changed in V4
    for(int r=0; r<high; r++){
        gameMap[r][0].symbol = wall1;
        gameMap[r][0].blocked=1;
        gameMap[r][width -1].symbol = wall1;
        gameMap[r][width -1].blocked=1;
    }
    for(int c=0; c<width; c++){
        gameMap[0][c].symbol = wall2;
        gameMap[0][c].blocked=1;
        gameMap[high -1][c].symbol = wall2;
        gameMap[high -1][c].blocked=1;
    }
}


int placeItem(ItemType t) {
    for(int i=0; i<30; i++){
        if(items[i].active==0){
            int rx, ry;
            while(1){
                rx=rand()%(high -2)+1;
                ry=rand()%(width-2)+1;
                if(gameMap[rx][ry].blocked==0 &&
                   gameMap[rx][ry].iii<0 &&
                   gameMap[rx][ry].enemyIndex<0){
                    break;
                }
            }
            items[i].active=1;
            items[i].x=rx;
            items[i].y=ry;
            items[i].NumberOf=0;
            gameMap[rx][ry].iii=i;
            items[i].type=t;
            return i;
        }
    }
    return -1;
}

// ---------------------------------------------------------------------
// player function is ready...
// ---------------------------------------------------------------------// ---------------------------------------------------------------------
// currentWeapon Option added newly... Worked Succesfully
// ---------------------------------------------------------------------// ---------------------------------------------------------------------
// changed...
// ---------------------------------------------------------------------


void start_player() {
    // best place for the @ Position in in the middle 
    // do not use your old alghorhitm
    player.x = high /2;
    player.y = width/2;
    player.health=30;
    player.maxHealth=30;
    player.gold=0;
    player.score=0;
    player.speed=1;
    player.damage=2;

    //mace should be infinite!!!
    player.currentWeapon.wtype=mace;
    player.currentWeapon.NumberOf=9999;
// Old Temple Map!!!! file processing go on .dat file else use template file in /Users/arminfarsi/Desktop/Final_Project/readyTemp.dat
    player.weaponCount=0;

    if(gameMap[player.x][player.y].blocked==1){
        int found=0;
        for(int i=0; i<high && found==0; i++){
            for(int j=0; j<width && found==0; j++){
                if(gameMap[i][j].blocked==0){
                    player.x=i;
                    player.y=j;
                    found=1;
                }
            }
        }
    }
}


void start_enemy() {
    srand(time(NULL));
    for(int i=0; i<30; i++){
        enemies[i].active=0;
    }
    int numEnemies=5;
    for(int i=0; i<numEnemies; i++){
        EnemyType et=(EnemyType)(rand()%5);
        int rx, ry;
        while(1){
            rx=rand()%(high-2)+1;
            ry=rand()%(width-2)+1;
            if(gameMap[rx][ry].blocked==0 &&
               gameMap[rx][ry].enemyIndex<0 &&
               !(rx==player.x && ry==player.y)){
                break;
            }
        }
        enemies[i].active=1;
        enemies[i].type=et;
        enemies[i].x=rx;
        enemies[i].y=ry;
        enemies[i].canMove=1;
        switch(et){
            case eneDaemon:
                enemies[i].health=10; enemies[i].damage=5;  break;
            case eneFire:  
                enemies[i].health=15; enemies[i].damage=10; break;
            case eneGiant:
                enemies[i].health=20; enemies[i].damage=15; break;
            case eneSnake:
                enemies[i].health=25; enemies[i].damage=20; break;
            case eneUndead:
                enemies[i].health=30; enemies[i].damage=30; break;
        }
        gameMap[rx][ry].enemyIndex=i;
    }
}


void strat_items() {
    for(int i=0; i<30; i++){
        items[i].active=0;
    }

//changed.

//changed...

    int numberofgold=5;
    int numberoftrap=5;
    int numberoffood=5;
    int numberofweap=3;
    int numberofspell=2;

    srand(time(NULL));

    while(numberofgold>0){
        int idx=placeItem(ITEM_GOLD);
        if(idx>=0){
            items[idx].subtype.goldAmount=10;
        }
        numberofgold--;
    }
    while(numberoftrap>0){
        placeItem(ITEM_TRAP);
        numberoftrap--;
    }
    while(numberoffood>0){
        int idx=placeItem(ITEM_FOOD);
        if(idx>=0){
            /* FOOD_NORMAL-> normal, dont used really!!!!!! ... */

            FoodType ft=(FoodType)(rand()%4);
            items[idx].subtype.foodType=ft;
        }
        //number of food decrease......
        numberoffood--;
    }
    while(numberofweap>0){
        int idx=placeItem(ITEM_WEAPON);
        if(idx>=0){
            int r=1+rand()%4;
            //ok.
            WeaponType wt=(WeaponType)r;
            items[idx].subtype.wtype=wt;
            switch(wt){
                case dagger:
                 items[idx].NumberOf=10; break;
                case wand:
                   items[idx].NumberOf=8;  break;
                case arrow:
                  items[idx].NumberOf=20; break;
                case sword:
                  items[idx].NumberOf=1;  break;
                default:
                     break; 
            }
        }
        numberofweap--;
    }
    while(numberofspell>0){
        int idx=placeItem(ITEM_SPELL);
        if(idx>=0){
            SpellType st=(SpellType)(rand()%3); 
            items[idx].subtype.spellType=st;
        }
        numberofspell--;
    }
}

// ---------------------------------------------------------------------
// رسم نقشه و موجودات در پنجره mapWin
// ---------------------------------------------------------------------
void drawMap(WINDOW *win) {
    werase(win); 
    for(int r=0; r<high; r++){
        for(int c=0; c<width; c++){
            char ch = gameMap[r][c].symbol;
            short colorPair = Cfloor;

            //به فایل گرافیک بازیکنان در Document/Graphics.txt
            // مراجحعه شود حتما
            // chnaged...
            //

//choosing color for wall1 , wall2 ....
            if(ch==wall1 || ch==wall2 || ch==windows){
                colorPair = Cwall;
            } else if(ch==flooor){
                colorPair = Cfloor;
            } else if(ch==RahRo){
                colorPair = Cwall;
            } else if(ch==pillar){
                colorPair = C2Items;
            }

///player colot
            if(r==player.x && c==player.y){
                ch=gamer;
                colorPair=Cgamer;
            }


            int eIdx=gameMap[r][c].enemyIndex;
            if(eIdx>=0 && enemies[eIdx].active==1){
                switch(enemies[eIdx].type){
                    case eneDaemon: ch='D'; break;
                    case eneFire:   ch='F'; break;
                    case eneGiant:  ch='G'; break;
                    case eneSnake:  ch='S'; break;
                    case eneUndead: ch='U'; break;
                }
                colorPair=CEnemy;
            }


            int iIdx=gameMap[r][c].iii;
            if(iIdx>=0 && items[iIdx].active==1 && !(r==player.x && c==player.y)){
                switch(items[iIdx].type){
                    case ITEM_GOLD:   
                    ch='$'; colorPair=Cgold;  
                    break;
                    case ITEM_TRAP:
                       ch='^'; 
                       colorPair=Ctrap;
                    break;
                    case ITEM_FOOD:   
                        ch='F';
                        colorPair=Cfood;  
                    break;
                    case ITEM_WEAPON:
                     ch='W'; colorPair=C2Items;
                     break;
                    case ITEM_SPELL: 
                     ch='*'; 
                     colorPair=C2Items;
                     break;
                }
            }

            wattron(win, COLOR_PAIR(colorPair));
            mvwaddch(win, r, c, ch);
            wattroff(win, COLOR_PAIR(colorPair));
        }
    }
    wrefresh(win);
}

// ---------------------------------------------------------------------
// //saving the games...
// saved temples are on /Documents/SaveDRogueGames.txt !>.
//chnaged.
/////////////////////////////
// ---------------------------------------------------------------------
void saveGame() {
    FILE *fp = fopen("savegame.dat","wb");
    if(!fp) return;
    fwrite(&player, sizeof(Player), 1, fp);
    fclose(fp);
}

// 
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ready Function For weapon inverntory
// REfined from V2

void addWeaponToInventory(WeaponType wt, int num) {
    for(int i=0; i<player.weaponCount; i++){
        if(player.inventory[i].wtype==wt){
            player.inventory[i].NumberOf += num;
            return;
        }
    }
    if(player.weaponCount<10){
        player.inventory[player.weaponCount].wtype=wt;
        player.inventory[player.weaponCount].NumberOf=num;
        player.weaponCount++;
    }
}

/* weapon_presenting */





void weapon_presenting(WINDOW *msgwin) {
    werase(msgwin);
    wprintw(msgwin, "=== Weapons Inventory ===\n");
    if(player.weaponCount == 0){
        wprintw(msgwin, "You have no weapons in your inventory.\n");
        wprintw(msgwin, "Press any key to continue...\n");
        wrefresh(msgwin);
        getch();
        return;
    }

    wprintw(msgwin, "[Melee Weapons]:\n");
    char letter = 'a';
    int indexMap[10];
    int countShown=0;

    // ابتدا سلاح‌های نزدیک‌بُرد
    for(int i=0; i<player.weaponCount; i++){
        WeaponType wt = player.inventory[i].wtype;
        int num = player.inventory[i].NumberOf;
        if(!Far_check(wt)){ 
            int dmg = getWeaponDamage(wt);
            int rng = getWeaponRange(wt);
            wprintw(msgwin, "  %c) ", letter);
            if(wt==mace)  wprintw(msgwin, "Mace");
            else          wprintw(msgwin, "Sword");
            wprintw(msgwin, " (Damage=%d, Range=[3x3 area], Qty=%d)\n", dmg, num);
            indexMap[countShown] = i;
            letter++;
            countShown++;
        }
    }

    wprintw(msgwin, "\n[Ranged Weapons]:\n");
    int rangedStartIndex = countShown;
    for(int i=0; i<player.weaponCount; i++){
        WeaponType wt = player.inventory[i].wtype;
        int num = player.inventory[i].NumberOf;
        if(Far_check(wt)==1){
            int dmg = getWeaponDamage(wt);
            int rng = getWeaponRange(wt);
            wprintw(msgwin, "  %c) ", letter);
            switch(wt){
                case dagger: 
                wprintw(msgwin, "Dagger"); 
                break;
                case wand:   
                wprintw(msgwin, "Wand");   
                break;
                case arrow:  
                wprintw(msgwin, "Arrow");  
                break;
                default: break;
            }
            wprintw(msgwin, " (Damage=%d, Range=%d, Qty=%d)\n", dmg, rng, num);
            indexMap[countShown] = i;
            letter++;
            countShown++;
        }
    }

    wprintw(msgwin, "\nPress letter to set that weapon as default (only if current weapon= Mace)\n");
    wprintw(msgwin, "Erroorororr..Crashed because Deafult Weapon Chaaaaangedddd. Press any other key to cancel.\n");

    wrefresh(msgwin);

    int ch = getch();
    int chosen = ch - 'a';
    if(chosen<0 || chosen>=countShown){
        wprintw(msgwin, "Invalid !!!!!!!!\nPress any key...\n");
        wrefresh(msgwin);
        getch();
        return;
    }

    if(player.currentWeapon.wtype != mace){
        wprintw(msgwin, "First Please  add your old weapon in the inventory!\nMenu closed.\n");
        wrefresh(msgwin);
        getch();
        return;
    }

    int invIndex = indexMap[chosen];
    Weapon *wp = &player.inventory[invIndex];
    if(wp->NumberOf <= 0){
        wprintw(msgwin, "Numbers=0, can't collect...\n");
        wrefresh(msgwin);
        getch();
        return;
    }

    player.currentWeapon = *wp;
    wprintw(msgwin, "Default weapon changed to: ");
    switch(wp->wtype){
        case mace:  
         wprintw(msgwin, "Mace");   break;
        case sword:  
        wprintw(msgwin, "Sword");   break;
        case dagger: 
        wprintw(msgwin, "Dagger"); break;
        case wand:   
        wprintw(msgwin, "Wand");   break;
        case arrow:  
        wprintw(msgwin, "Arrow");  break;
    }
    wprintw(msgwin, "\nplease Press any key...\n");
    wrefresh(msgwin);

    getch();
}

// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
//Items Collecting
// Changed...
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
void pickupItem(int iIdx, WINDOW *msgwin) {
    if(items[iIdx].active==0) return;
    switch(items[iIdx].type){
        case ITEM_GOLD:
            player.gold += items[iIdx].subtype.goldAmount;
            player.score += items[iIdx].subtype.goldAmount;
            wprintw(msgwin, "You picked up %d gold.\n", items[iIdx].subtype.goldAmount);
            break;
        case ITEM_TRAP:
            wprintw(msgwin, "Ohhhhhhhhhhhhhhhh! A trap! You lose 5 HP.\n");
            player.health -= 5;
            break;
        case ITEM_FOOD: {
            FoodType ft = items[iIdx].subtype.foodType;
            switch(ft){
                case normal:
                    wprintw(msgwin, "A normal food: +3 HP.\n");
                    player.health += 3;
                    if(player.health>player.maxHealth) player.health=player.maxHealth;
                    break;
                case advanced:
                    wprintw(msgwin, "An advanced meal: +5 HP and You have short power booooooost.\n");
                    player.health += 5;
                    if(player.health>player.maxHealth) player.health=player.maxHealth;
                    player.damage += 3;
                    break;
                case magic:
                    wprintw(msgwin, "A magic food: +3 HP and You Have short speed booooooooooost.\n");
                    player.health += 3;
                    if(player.health>player.maxHealth) player.health=player.maxHealth;
                    player.speed += 1;
                    break;
                case rotten:
                    wprintw(msgwin, "Rotten food: -5 HP!\n");
                    player.health -= 5;
                    break;
            }
        } break;
        case ITEM_WEAPON: {
            WeaponType wt = items[iIdx].subtype.wtype;
            int num = items[iIdx].NumberOf;
            addWeaponToInventory(wt, num);
            wprintw(msgwin, "You collected a weapon, qty=%d\n", num);
        } break;
        case ITEM_SPELL: {
            SpellType st = items[iIdx].subtype.spellType;
            wprintw(msgwin, "You collected a spell: ");
            switch(st){
                case health:
                    wprintw(msgwin, "Health Spell (+10 HP).\n");
                    player.health += 10;
                    if(player.health>player.maxHealth) player.health=player.maxHealth;
                    break;
                case speed:
                    wprintw(msgwin, "Speed Spell (+1 speed).\n");
                    player.speed += 1;
                    break;
                case damage:
                    wprintw(msgwin, "Damage Spell (+5 damage).\n");  
                    player.damage += 5; 
                    break;
            }
        } break;
    }
    items[iIdx].active=0;
    gameMap[items[iIdx].x][items[iIdx].y].iii=-1;
}


void checkCellEvent(int nx, int ny, WINDOW *msgwin) {
    int iIdx = gameMap[nx][ny].iii;

    int Id_akhari2 = gameMap[nx][ny].iii;

    if(iIdx>=0 && items[iIdx].active==1){

        pickupItem(iIdx, msgwin);
    }
}

// ---------------------------------------------------------------------
// checked The type of Your Weapon.......
// ---------------------------------------------------------------------




void areaMeleeAttack(WINDOW *msgwin, int dmg) {
    int px = player.x;
    int py = player.y;
    wprintw(msgwin, "You do an area melee(NEARBY ATTACK) attack (damage=%d) around you!\n", dmg);

//nt code
    for(int dx=-1; dx<=1; dx++){
        for(int dy=-1; dy<=1; dy++){
            if(dx==0 && dy==0) 
                continue;
            int nx = px+dx;
            int ny = py+dy;
            if(nx<0||nx>=high||ny<0||ny>=width) 
                continue;
            int eIdx = gameMap[nx][ny].enemyIndex;
            if(eIdx>=0 && enemies[eIdx].active==1){
                enemies[eIdx].health -= dmg;
                wprintw(msgwin, " - Damaged enemy at [%d,%d]. ", nx, ny);
                if(enemies[eIdx].health<=0){
                    wprintw(msgwin, "Enemy killed!\n");
                    player.score += 10;
                    gameMap[nx][ny].enemyIndex = -1;
                    enemies[eIdx].active=0;
                } else {
                    wprintw(msgwin, "(Remain HP=%d)\n", enemies[eIdx].health);
                }
            }
        }
    }
}

void rangedSingleTargetAttack(WINDOW *msgwin, int range, int dmg) {

    int target=-1;
    float minDist=9999999.0f;
    int px=player.x, py=player.y;

    for(int i=0; i<30; i++){
        if(enemies[i].active==0) continue;
        int ex=enemies[i].x;
        int ey=enemies[i].y;
        float dist=sqrtf((ex-px)*(ex-px)+(ey-py)*(ey-py));
        if(dist<=range && dist<minDist){
            minDist=dist;
            target=i;
        }
    }

    if(target<0){
        wprintw(msgwin, "No enemy in range (<=%d)!\n", range);
        return;
    }
    wprintw(msgwin, "You do a ranged attack (damage=%d) on enemy.\n", dmg);
    enemies[target].health -= dmg;
    if(enemies[target].health<=0){
        wprintw(msgwin, "Enemy slain!\n");
        player.score += 10;
        gameMap[enemies[target].x][enemies[target].y].enemyIndex=-1;
        enemies[target].active=0;
    } else {
        wprintw(msgwin, "Enemy HP after hit = %d\n", enemies[target].health);
    }
}


void performAttack(WINDOW *msgwin) {
    WeaponType wt = player.currentWeapon.wtype;
    int dmg = getWeaponDamage(wt);

    if(Far_check(wt)==1){
        if(player.currentWeapon.NumberOf<=0){
            wprintw(msgwin, "No MONESTER!!!! for your ranged weapon!\n");
            return;
        }
    }

    switch(wt){
        case mace:
            areaMeleeAttack(msgwin, dmg);
            break;
        case sword:
            areaMeleeAttack(msgwin, dmg);
            break;
        case dagger:
            rangedSingleTargetAttack(msgwin, 5, dmg);
            player.currentWeapon.NumberOf--;
            break;
        case wand:
            rangedSingleTargetAttack(msgwin, 15, dmg);
            player.currentWeapon.NumberOf--;
            break;
        case arrow:
            rangedSingleTargetAttack(msgwin, 5, dmg);
            player.currentWeapon.NumberOf--;
            break;
    }
}

//player move and Enemies...
//Changed From V4

void movePlayer(int input, WINDOW *msgwin) {
    int dx=0, dy=0;
    switch(input){
        case KEY_UP: dx=-1;  
        break;
        case KEY_DOWN: dx=+1;  
        break;
        case KEY_LEFT: dy=-1; 
        break;
        case KEY_RIGHT:dy=+1; 
        break;
        default: break;
    }
    int nx=player.x+dx;
    int ny=player.y+dy;
    if(nx<0||nx>=high||ny<0||ny>=width) return;
    if(gameMap[nx][ny].blocked==1){
        wprintw(msgwin, "Blocked by a wall.\n");
        return;
    }
    player.x=nx;
    player.y=ny;
    checkCellEvent(nx,ny,msgwin);
}

void moveEnemies(WINDOW *msgwin) {
    for(int i=0; i<30; i++){
        if(enemies[i].active==0) 
            continue;
        if(enemies[i].canMove==0) 
            continue;
        int distX=player.x - enemies[i].x;
        int distY=player.y - enemies[i].y;
        int stepX=(distX>0)?1:((distX<0)?-1:0);
        int stepY=(distY>0)?1:((distY<0)?-1:0);
        int newX=enemies[i].x+stepX;
        int newY=enemies[i].y+stepY;

        if(newX==player.x && newY==player.y){
            wprintw(msgwin, "Enemy hitttss you for %d damage!\n", enemies[i].damage);
            player.health-=enemies[i].damage;
            if(player.health<=0){
                wprintw(msgwin, "You have been slainnnnnnnn!\n");
            }
        } else {
            if(gameMap[newX][newY].blocked==0 &&
               gameMap[newX][newY].enemyIndex<0 &&
               !(newX==player.x && newY==player.y)){
                gameMap[enemies[i].x][enemies[i].y].enemyIndex=-1;
                enemies[i].x=newX;
                enemies[i].y=newY;
                gameMap[newX][newY].enemyIndex=i;
            }
        }
    }
}







//Asl kar

void gameLoop(WINDOW *mapwin, WINDOW *msgwin) {
    int ch;
    int running=1; 
    while(running==1){
        werase(msgwin);

        wprintw(msgwin, "Player: %s | HP:%d/%d | Gold:%d | Score:%d\n",
                currentUser.username, player.health, player.maxHealth,
                player.gold, player.score);

        WeaponType wt=player.currentWeapon.wtype;
        wprintw(msgwin, "Current Weapon: ");
        switch(wt){
            case mace:   
            wprintw(msgwin,"Mace");   
            break;
            case dagger: 
            wprintw(msgwin,"Dagger"); 
            break;
            case wand:   
            wprintw(msgwin,"Wand");   
            break;
            case arrow:  
            wprintw(msgwin,"Arrow");  
            break;
            case sword:  
            wprintw(msgwin,"Sword");  
            break;
        }
        wprintw(msgwin, " (qty:%d)\n", player.currentWeapon.NumberOf);

        wprintw(msgwin, "[Arrows] Move | 'a' Attack | 'i' Inventory | 's'  Save :))))) | 'q' Quitt :((((\n");
        wrefresh(msgwin);

        drawMap(mapwin);

        if(player.health<=0){
            wprintw(msgwin, "You are dead!!!!!. Game Over.\n");
            wrefresh(msgwin);
            getch();
            break;
        }

        ch=getch();
        switch(ch){
            case 'q':
                running=0; 
                break;
            case 'a':
                performAttack(msgwin);
                break;
            case 's':
                saveGame();
                wprintw(msgwin,"Game saved.\n");
                break;
            case 'i':
               //changed !
                weapon_presenting(msgwin);
                break;
            default:
                movePlayer(ch,msgwin);
                break;
        }
        moveEnemies(msgwin);
        wrefresh(msgwin);

        if(player.gold>=50){
            wprintw(msgwin,"You found enough gold. You win!\n");
            wrefresh(msgwin);
            getch();
            running=0;
        }
    }
}

// ---------------------------------------------------------------------
// ثبت امتیازات (scores.dat) و نمایش آن
//another sample filed created  at /Users/arminfarsi/Document/SAMPLEFOsaved.txt
//changed.
// ---------------------------------------------------------------------
typedef struct {
    char username[50];
    int score;
} ScoreRecord;


//file process
//nt by me 


void addScore(const char *uname,int sc) {
    FILE *fp=fopen("scores.dat","ab");
    if(!fp)return;
    ScoreRecord r;
    strcpy(r.username,uname);
    r.score=sc;
    fwrite(&r,sizeof(r),1,fp);
    fclose(fp);
}

void showHighScores_ncurses() {
    clear();
    mvprintw(0,0,"=== High Scores ===");
    ScoreRecord arr[100];
    int count=0;
    FILE *fp=fopen("scores.dat","rb");
    if(fp){
        while(fread(&arr[count],sizeof(ScoreRecord),1,fp)==1 && count<100){
            count++;
        }
        fclose(fp);
    }
    //bubble sort now
    // might be chnaged TO quick sort
    //...

    for(int i=0;i<count;i++){
        for(int j=i+1;j<count;j++){
            if(arr[j].score>arr[i].score){
                ScoreRecord tmp=arr[i];
                arr[i]=arr[j];
                arr[j]=tmp;
            }
        }
    }
    int lim=(count<10?count:10);

    for(int i=0;i<lim;i++){
        if(i==0){
            attron(COLOR_PAIR(Cgamer));
            mvprintw(i+1,0,"%d) %s - %d [LEGEND]",i+1,arr[i].username,arr[i].score);
            attroff(COLOR_PAIR(Cgamer));
        } else if(i==1){
            attron(COLOR_PAIR(Cgold));
            mvprintw(i+1,0,"%d) %s - %d [GOAT]",i+1,arr[i].username,arr[i].score);
            attroff(COLOR_PAIR(Cgold));
        } else if(i==2){
            attron(COLOR_PAIR(CEnemy));
            mvprintw(i+1,0,"%d) %s - %d [MASTER]",i+1,arr[i].username,arr[i].score);
            attroff(COLOR_PAIR(CEnemy));
        } else {
            mvprintw(i+1,0,"%d) %s - %d",i+1,arr[i].username,arr[i].score);
        }
    }
    mvprintw(lim+2,0,"Press any key...");
    refresh();
    getch();
}

// ---------------------------------------------------------------------
// Main Menu Of Game
// Copied from V3
//Not chnaged.
// ---------------------------------------------------------------------

void mainMenu() {
    while(1){
        clear();
        mvprintw(0,0,"=== MAIN MENU ===");
        mvprintw(1,0,"1) Register");
        mvprintw(2,0,"2) Login");
        mvprintw(3,0,"3) Play as guest");
        mvprintw(4,0,"4) High scores");
        mvprintw(5,0,"5) Exit");
        refresh();
        int c=getch();
        switch(c){
            case '1':
                clear();
                Sign_up();
                break;
            case '2':
                clear();
                if(login()==1){
                    return;
                }
                break;
            case '3':
                strcpy(currentUser.username,"Guest");
                return;
            case '4':
                showHighScores_ncurses();
                break;
            case '5':
                endwin();
                exit(0);
            default:
                break;
        }
    }
}

// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------MAIN--------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------

int main(){
    initscr();
    cbreak();
    noecho();
    keypad(stdscr,1);
    curs_set(0);


//use for only Color Pair checking
//for sure
//Internet

    if(has_colors()){
        start_color();
        init_pair(Cwall,   COLOR_BLUE,   COLOR_BLACK);
        init_pair(Cfloor,  COLOR_WHITE,  COLOR_BLACK);
        init_pair(Cgamer,  COLOR_YELLOW, COLOR_BLACK);
        init_pair(CEnemy,  COLOR_RED,    COLOR_BLACK);
        init_pair(Cgold,   COLOR_YELLOW, COLOR_BLACK);
        init_pair(Ctrap,   COLOR_MAGENTA,COLOR_BLACK);
        init_pair(Cfood,   COLOR_GREEN,  COLOR_BLACK);
        init_pair(C2Items, COLOR_CYAN,   COLOR_BLACK);
    }

    mainMenu();

    srand(time(NULL));
    generateMap();
    start_player();
    start_enemy();
    strat_items();
//refer to line 893 
//changed the pointer WINDOWS!!!!!
//not changed from V3

    WINDOW *mapWin=newwin(high,width,0,0);
    WINDOW *msgWin=newwin(8,width,high,0);

    gameLoop(mapWin,msgWin);

    if(player.score>0 && strcmp(currentUser.username,"Guest")!=0){
        addScore(currentUser.username,player.score);
    }

    delwin(mapWin);
    delwin(msgWin);
    endwin();
    printf("HORRAYYY!!!!!!!Game finished!!!!!!!!!!! Final Score: %d\n",player.score);
    return 0;
}
