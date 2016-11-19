
//To compile: gcc -o bs.exe bs.c ADT/matriks.c ADT/stackt.c ADT/mesinkar.c ADT/point.c ADT/queue.c ADT/listlinier.c ADT/mesinkata.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "ADT/mesinkar.h"
#include "ADT/mesinkata.h"
#include "ADT/matriks.h"
#include "ADT/stackt.h"
#include "ADT/point.h"
#include "ADT/queue.h"
#include "ADT/listlinier.h"
#include "ADT/Stats_infotype.h"
#include "ADT/pohon.h"
#include "ADT/stringMandiri.h"
#include <string.h>

#define newLine printf("\n")
#define pressEnterPrompt printf("Press enter to continue...\n");
#define debug_test printf("Di sini!\n")

//Variabel global
FILE *player_data;
FILE *enemy_data;
Queue enemy_command;
List player_command;
TriTree skill_tree;
MATRIKS current_map;
POINT player_location;
boolean logged_in;
boolean battle_over;
boolean enemy_dead;
boolean player_dead;
boolean player_win;
boolean player_command_done;
int player_HP_current;
int player_MP_current;
int enemy_HP_current;
int count;
int lvl_up_req;
char mod[15];

STATS player_current;
STATS player__passive_modifier;
STATS player__active_modifier;
STATS enemy_current;

//Menu Utama
int main_menu();
void new_game();
void start_game();

//Loading
void load_player_stats (STATS *player_current,char* file_name);
void load_enemy (STATS *enemy_current, char* enemy_file);
MATRIKS load_map(char map_name[]);
void load_update_map(MATRIKS map);
void load_player_skill(TriTree *P, char* file_name);

//Pertarungan
char enemy_command_generate ();
void battle_interaction(List player_command, Queue enemy_command, STATS *player_stats, STATS *enemy_current);
void battle_transition (char* map_name, int x, int y);
void Get_Exp(STATS *player_current, int enemy_exp);
int lvl_up(int current_level);

//Lain-lain
void player_stats_tulis();
int abszero(int x);
void player_get_medicine (int *HP, int* MP, int MaxHP, int MaxMP);

//player
void player_choose_skill();
void player_show_skills (TriTree P);


/***************************	FUNGSI UTAMA	**************************/
int main() 
{
	srand(time(NULL));
	int menu_choice;
	boolean exit_menu = false;
	//MakeMATRIKS (100,100, &current_map);
	
	/*
	char test1[max_name];
	char test2[max_name];
	
	string_copy (test1, "maphubx");
	string_copy (test2, ".txt");
	string_concat (test1, test2);
	printf ("%s\n",test1);
	*/
	logged_in = false;	
	while (!exit_menu)	
	{
		menu_choice = main_menu();
		
		switch(menu_choice)
		{
			case 1: new_game();
					break;
			case 2:
					if (logged_in)
					{
						player_win = false;
						player_dead = false;
						
						start_game();					
					}else
					{
						printf ("Not logged in yet!");
						newLine;
					}
					break;
			case 4:	exit_menu = true;
					break;
			
		}
	}	
	return 0;
}

//////////////////////////	Implementasi Fungsi dan Prosedur	/////////////////////////////


/********************************	MENU DAN BADAN GAME		**********************************************/
int main_menu()
/*
	Menampilkan pilihan menu di layar dan menunggu input pemain. Input yang tersedia adalah (1) New Game, (2) Start Game,
	(3) Load, (4) Exit. Input pemain akan dikembalikan ke program utama.
*/
{	
	int option;
	newLine;newLine;newLine;
	
	printf (" ****************"); newLine;
	printf ("*| BRIGHT SOULS |*"); newLine;
	printf (" ****************"); newLine;
	
	if (logged_in)
	{
		printf ("Logged in as: %s", player_current.name);newLine;
		player_stats_tulis(); newLine;
	}
	newLine;

	printf ("1. New Game");newLine;
	printf ("2. Start Game");newLine;
	printf ("3. Load Game");newLine;
	printf ("4. Exit");newLine;
	newLine;
	do
	{
		printf("Input: "); 	scanf("%d", &option);getchar();
	}while (option != 1 && option != 2 && option != 3 && option != 4);
	newLine;
	
	return option;
}
	
void new_game() 
/*
	Menerima input nama pemain baru dan membuat file eksternal sesuai dengan input nama.
	I.S.		:	player_name sembarang.
					logged_in = false.
					
	F.S.		:	Jika nama sudah pernah dipakai maka input ditolak dan program
					kembali ke main_menu tanpa mengubah apapun.
					
					Jika nama belum pernah dipakai akan dibuat file eksternal dengan nama <player_name>.txt yang berisi
					stats awal (LVL = 1, HP = 100, MP = 10, STR = 10, DEF = 5, EXP = 0) dan logged_in = true. File ini 
					kemudian akan di load oleh fungsi load_player_stats untuk digunakan dalam permainan baru.
*/
{
	char player_name[max_name] ;
	char name[max_name];
	
	printf ("Enter your name: "); scanf ("%s", player_name); getchar();

	string_copy(name, player_name);
	
	if (access(player_name, F_OK) != -1) //Mengecek keberadaan file eksternal.
	{
		printf ("Name already exist!");
	}else //File eksternal belum ada
	{

		//Membuat file eksternal dengan stats.
		string_concat (name, " 1 100 10 10 5 0 0."); 

		player_data = fopen (player_name, "a+");
			fputs (name, player_data);
		fclose(player_data);
		
		//Load stats player
		load_player_stats(&player_current, player_name); 
		
		//Mengubah nilai logged_in.
		logged_in = true;

		string_copy(player_name, player_current.name);
		string_concat (player_name, "_skill.txt");

		player_data = fopen (player_name, "a+");
			fputs ("(God's_Blessing|0|2(Warrior's_Strength|0|3(Warcry|0|4()()())(Backstab|0|4()()())(Disarm|0|4()()()))()(Knight's_Fortitude|0|3(6th_Sense|0|4()()())(Riposte|0|4()()())()).", player_data);
		fclose(player_data);
		load_player_skill(&skill_tree, player_name);

		
	};
	newLine;
}

void start_game()
/*
	Prosedur yang menjadi badan utama dari permainan. Di sini (1) map akan diload, (2) pemain bisa bergerak, 
	(3) transisi ke layar pertarungan, (4) Menampilkan posisi pemain dalam matriks map.
	
	I.S.		:	player_input sembarang.
					current_map_name = "map_hubx.txt"
					player_stats terdefinisi.
					
	F.S.		:	Jika player_input = quit, maka akan kembali ke main_menu. logged_in = false.
	
					***Jika player_input = save maka akan dilakukan fungsi save.
					
					***Jika player_input = 'level up' maka jika player_upgrade_point > 0 akan ditampilkan pilihan skill yang 
					bisa diambil. Tapi jika player_upgrade_point = 0, ditampilkan pesan "Tidak ada upgrade point!".
					METODE PENYIMPANAN SKILL BELUM DITENTUKAN.
						***Skill yang diambil bisa berupa tambahan stats permanen dan atau skill aktif. APAKAH POHON SKILL AKTIF
						DAN PASIF DISAMAKAN ATAU DIBEDAKAN BELUM TAU.
							***Ide skill aktif:
								-War Cry (Stats modifier, MP = 2, tier 1): Increases player attack by 3*player_level for 2 rounds.
								-Sand Attack (Stats modifier, Mp = 1, tier 1): Reduces enemy defense by 2*player_level for 1 round.
								-Fortify (Stats modifier, Mp= 2, tier 1): Increases player defense by 3*player_level for 2 rounds.
								
								-Smoke screen (Block Modifier, MP = 3, tier 2): Dodge enemy attack regardless of A or F.
								-Disarm (Attack Modifier, MP = 3, tier 2): Receive 50% damage in A vs A clash.
								-6th sense (Flank Modifier, MP = 3, tier 2): Receive 50% damage in F vs F clash.
							
								-Piercing attack (Attack Modifier, MP = 5, tier 3): Ignore 100% enemy defense vs A or B, but will receive 50% extra damage if flanked.
								-Backstab (Flank Modifier, MP = 5, tier 3): Deal 200% extra damage vs F or B, but will receive 50% extra damage if attacked.
								-Riposte (Block Modifier, MP = 5, tier 3): Deal 200% extra damage vs A or B, but will receive 50% extra damage if flanked  
						
					***Jika player input = 'skill' maka akan ditampilkan daftar skill pasif dan aktif yang sudah diambil 
					pemain. 
	
					Jika player_input = GU, GR, GD, atau GL maka ikon player (@) akan bergerak selama petak yang dituju
					adalah spasi (' '). Petak saat ini akan diubah menjadi ' ' dan petak tujuan diubah menjadi '@' sehingga
					pemain 'seolah-olah' bergerak.
					TETAPI jika petak yang dituju adalah 'E', maka akan terjadi battle_transition. Jika pemain atau musuh 
					mati maka battle_over = true dan diberikan pesan yang sesuai ("Anda mati" atau "Musuh mati"). Jika player 
					mati maka player_dead = true. Jika musuh yang mati maka petak saat ini akan diubah menjadi ' ' dan petak
					tujuan diubah menjadi '@' sehingga pemain 'seolah-olah' bergerak dan E dihapus dari peta.
					***TETAPI jika petak yang dituju berisi 'M', maka HP player akan disembuhkan sebanyak 25% maksimum HP 
					   dan MP sebanyak 20% maksimum MP.
					***TETAPI jika petak yang dituju adalah '/' maka akan terjadi transisi ke map lain dengan menerapkan graf.
					
					
					Jika player_dead = true atau player_win = true, maka akan ditampilkan pesan yang sesuai("anda mati" atau ***CREDITS) 
					dan kembali ke main_menu. logged_in = false;
					
					***CREDITS jika pemain menang
*/
{
	char player_input[10];
	char current_map_name[100] = "map_hubx.txt";
	char skill_choice [50];
	//Load map pertama.
	current_map = load_map(current_map_name);
	//printf ("%d %d\n", NBrsEff(current_map), NKolEff(current_map));
	
	string_copy (player_input, "start");
	//Badan game; akan diulang selama syarat dipenuhi.
	while (!string_compare(player_input, "quit") && !player_dead && !player_win)
	{
		
		//Menulis posisi pemain pada matriks.
		TulisPOINT(player_location, NKolEff(current_map)-2); newLine;
		
		//Menulis status player ke layar.
		player_stats_tulis();newLine;
		
		//Menulis matriks map ke layar.
		TulisMATRIKS(current_map);
		
		//Menunggu input pemain
		printf("Input (help to show command list): ");
		scanf ("%s", player_input);
		getchar();
		
		//jalan ke kanan
		// Memastikan petak yang dituju tidak berisi '#'
		if ((Elmt (current_map,Ordinat(player_location), Absis(player_location)+1) != '#')	&& string_compare(player_input,"GR"))
			{
				//Mengecek apakah petak yang dituju spasi atau E.
				if (Elmt (current_map,Ordinat(player_location), Absis(player_location)+1) == ' ')
				{
					//Ilusi pemain bergerak
					Elmt(current_map, Ordinat(player_location), Absis(player_location)) = ' ';
					Elmt (current_map,Ordinat(player_location), Absis(player_location)+1) = '@';
					Absis(player_location)++;
				}else if (Elmt (current_map,Ordinat(player_location), Absis(player_location)+1) == 'E')
				{
					//Transisi ke layar pertarungan.
					battle_transition (current_map_name, Absis(player_location)+1,Ordinat(player_location)); 
					
					//Ilusi pemain bergerak dan E dihapus
					if (battle_over)
					{
						Elmt(current_map, Ordinat(player_location), Absis(player_location)) = ' ';
						Elmt (current_map,Ordinat(player_location), Absis(player_location)+1) = '@';
						Absis(player_location)++;
					}
				}else if (Elmt (current_map,Ordinat(player_location), Absis(player_location)+1) == 'M')
				{
					player_get_medicine(&player_HP_current, &player_MP_current, player_current.HP, player_current.MP);
					Elmt(current_map, Ordinat(player_location), Absis(player_location)) = ' ';
					Elmt (current_map,Ordinat(player_location), Absis(player_location)+1) = '@';
					Absis(player_location)++;
				}
			}
			
			//jalan ke kiri
			else if ((Elmt (current_map,Ordinat(player_location), Absis(player_location)-1) != '#') && string_compare(player_input,"GL"))
			{
				if (Elmt (current_map,Ordinat(player_location), Absis(player_location)-1) == ' ')
				{
					Elmt(current_map, Ordinat(player_location), Absis(player_location)) = ' ';
					Elmt (current_map,Ordinat(player_location), Absis(player_location)-1) = '@';
					Absis(player_location)--;
				}else if (Elmt (current_map,Ordinat(player_location), Absis(player_location)-1) == 'E')
				{
					battle_transition (current_map_name, Absis(player_location)-1,Ordinat(player_location)); 
					if (battle_over)
					{
						Elmt(current_map, Ordinat(player_location), Absis(player_location)) = ' ';
						Elmt (current_map,Ordinat(player_location), Absis(player_location)-1) = '@';
						Absis(player_location)--;
					}
				}else if (Elmt (current_map,Ordinat(player_location), Absis(player_location)-1) == 'M')
				{
					player_get_medicine(&player_HP_current, &player_MP_current, player_current.HP, player_current.MP);
					Elmt(current_map, Ordinat(player_location), Absis(player_location)) = ' ';
					Elmt (current_map,Ordinat(player_location), Absis(player_location)-1) = '@';
					Absis(player_location)--;
				}
			}
			//jalan ke bawah
			else if ((Elmt (current_map,Ordinat(player_location)+1, Absis(player_location)) != '#') && string_compare(player_input,"GD"))
			{
				if (Elmt (current_map,Ordinat(player_location)+1, Absis(player_location)) == ' ')
				{
					Elmt(current_map, Ordinat(player_location), Absis(player_location)) = ' ';
					Elmt (current_map,Ordinat(player_location)+1, Absis(player_location)) = '@';
					Ordinat(player_location)++;
				}else if (Elmt (current_map,Ordinat(player_location)+1, Absis(player_location)) == 'E')
				{
					battle_transition (current_map_name, Absis(player_location),Ordinat(player_location)+1); 
					if (battle_over)
					{
						Elmt(current_map, Ordinat(player_location), Absis(player_location)) = ' ';
						Elmt (current_map,Ordinat(player_location)+1, Absis(player_location)) = '@';
						Ordinat(player_location)++;
					}
				}else if (Elmt (current_map,Ordinat(player_location)+1, Absis(player_location)) == 'M')
				{
					player_get_medicine(&player_HP_current, &player_MP_current, player_current.HP, player_current.MP);
					Elmt(current_map, Ordinat(player_location), Absis(player_location)) = ' ';
					Elmt (current_map,Ordinat(player_location)+1, Absis(player_location)) = '@';
					Ordinat(player_location)++;
				}
			}
			
			//jalan ke atas
			else if ((Elmt (current_map,Ordinat(player_location)-1, Absis(player_location)) != '#') && string_compare(player_input,"GU"))
			{
				if (Elmt (current_map,Ordinat(player_location)-1, Absis(player_location)) == ' ')
				{
					Elmt(current_map, Ordinat(player_location), Absis(player_location)) = ' ';
					Elmt (current_map,Ordinat(player_location)-1, Absis(player_location)) = '@';
					Ordinat(player_location)--;
				}else if (Elmt (current_map,Ordinat(player_location)-1, Absis(player_location)) == 'E')
				{
					battle_transition (current_map_name, Absis(player_location),Ordinat(player_location)-1); 
					if (battle_over)
					{
						Elmt(current_map, Ordinat(player_location), Absis(player_location)) = ' ';
						Elmt (current_map,Ordinat(player_location)-1, Absis(player_location)) = '@';
						Ordinat(player_location)--;
					}
				}else if (Elmt (current_map,Ordinat(player_location)-1, Absis(player_location)) == 'M')
				{
					player_get_medicine(&player_HP_current, &player_MP_current, player_current.HP, player_current.MP);
					Elmt(current_map, Ordinat(player_location), Absis(player_location)) = ' ';
					Elmt (current_map,Ordinat(player_location)-1, Absis(player_location)) = '@';
					Ordinat(player_location)--;
				}
			}
		
		else if (string_compare(player_input,"quit"))
		{
			logged_in = false;
		}

		else if (string_compare(player_input,"give_me_level"))
		{
			printf("Level Up! Stats Upgraded:\n");
			printf ("1 upgrade point received!\n");
			player_current.EXP = 0;
			player_current.LVL++;
			player_current.HP+=25;
			player_current.MP+=5;
			player_current.STR+=3;
			player_current.DEF+=3;
			player_current.UP+=1;

			pressEnterPrompt;
			getchar();
		}

		else if (string_compare(player_input,"level_up"))
		{
			if (player_current.UP > 0)
			{
				printf("Skill yang dapat diambil: \n");
				PrintTreeAvailable (skill_tree, player_current.LVL);	
				newLine;

				printf ("Masukkan nama skill yang ingin diambil: ");
				scanf("%s", skill_choice); getchar();

				player_choose_skill (skill_choice, &player_current);

			}
			else 
			{
				printf ("Anda tidak punya upgrade point\n");
			}
			pressEnterPrompt;
			getchar();
		}

		else if (string_compare(player_input,"skills"))
		{
			PrintTreeTaken(skill_tree);
			pressEnterPrompt;
			getchar();
		}

		else if (string_compare(player_input, "help"))
		{
			printf("GR, GL, GU, GD\n");
			printf("level_up\n");
			printf("skills\n");
			printf("quit\n");

			pressEnterPrompt;
			getchar();

		}
		newLine;
		
		
	}
}

/////////////////////////////////	END		///////////////////////////////////////////////////////

/********************************	SEGALA TENTANG MAP		**********************************************/
MATRIKS load_map(char* map_name)
/*
	Membaca file eksternal yang berisi data peta yang berformat kotak. Akan dibaca perkarakter dan dimasukkan 
	ke matriks. Jika terbaca '|' maka row matriks ditambah satu. jika terbaca '@' maka akan dibuat Point sebagai 
	letak awal pemain. Jika terbaca '.' maka proses load peta berakhir. Akan dikembalikan Matriks yang isinya 
	data dari file eksternal.

	Contoh isi file eksternal (satu line panjaaaaaaaaang):
	###########///###############///##############|#                                            #|#                                            #|#      ###      ###      ###      ###        #|#      ###      ###      ###      ###        #|#                                            /|#  @     E                                   /|#                                            /|#      ###      ###      ###      ###        #|#      ###      ###      ###      ###        #|#                                            #|#                                            #|#                                            #|###########///###############///##############.
	
	Visual dari file eksternal:
	###########///###############///##############|
	#                                            #|
	#                                            #|
	#      ###      ###      ###      ###        #|
	#      ###      ###      ###      ###        #|
	#                                            /|
	# @     E                                    /|
	#                                            /|
	#      ###      ###      ###      ###        #|
	#      ###      ###      ###      ###        #|
	#                                            #|
	#                                            #|
	#                                            #|
	###########///###############///##############.
	
	Yang akan ditampilkan di layar:
	###########---###############---##############
	#                                            #
	#                                            #
	#      ###      ###      ###      ###        #
	#      ###      ###      ###      ###        #
	#                                             
	# @     E                                     
	#                                             
	#      ###      ###      ###      ###        #
	#      ###      ###      ###      ###        #
	#                                            #
	#                                            #
	#                                            #
	###########---###############---##############
*/
{
	int i,j;
	START(map_name);
	MATRIKS map;
	
	i = 1;
	
	//Menghitung ukuran efektif yang diperlukan matriks.
	while (!EOP)
	{
		j = 1;
		while (CC != '|' && !EOP)
		{
			if (CC == '@')
			{
				player_location = MakePOINT (j,i);
			}
			
			j++;
			ADV();
		}
		if (!EOP)
		{
			i++;
			ADV();
		}
	}
	
	//Mengisi matriks(i,j) dengan karakter yang terbaca.
	MakeMATRIKS (i, j-1, &map);
	START(map_name);
	i = 1;
	while (!EOP)
	{
		j = 1;
		while (CC != '|' && !EOP)
		{
			map.Mem[i][j] = CC;
			j++;
			ADV();
		}

		if (!EOP)
		{
			i++;
			ADV();
		}
	}
	return map;
}


/////////////////////////////////	END		///////////////////////////////////////////////////////

/********************************	SEGALA TENTANG PEMAIN		**********************************************/


void load_player_stats (STATS *player_current,char* file_name) //MUNGKIN BISA DIJADIKAN FUNGSI LOAD
/*
	Membaca stats player dari file eksternal.
	
	I.S.	:	<file_name>.txt pasti ada.
				player_current sembarang.
	F.S.	:	komponen player_current terdefinisi semua.
*/
{
	char buffer[100];
	
	STARTKATA(file_name);
	
	string_copy ((*player_current).name, CKata.TabKata);
	
	ADVKATA();
	string_copy (buffer, CKata.TabKata);
	(*player_current).LVL = atoi (buffer);
	
	ADVKATA();
	string_copy (buffer, CKata.TabKata);
	(*player_current).HP = atoi (buffer);
	player_HP_current = (*player_current).HP;
	
	ADVKATA();
	string_copy (buffer, CKata.TabKata);
	(*player_current).MP = atoi (buffer);
	player_MP_current = (*player_current).MP;
	
	ADVKATA();
	string_copy (buffer, CKata.TabKata);
	(*player_current).STR = atoi (buffer);
	
	ADVKATA();
	string_copy (buffer, CKata.TabKata);
	(*player_current).DEF = atoi (buffer);
	
	ADVKATA();
	string_copy (buffer, CKata.TabKata);
	(*player_current).EXP = atoi (buffer);

	ADVKATA();
	string_copy (buffer, CKata.TabKata);
	(*player_current).UP = atoi (buffer);
}

void player_stats_tulis()
/*
	Menulis komponen player.current.
	I.S.	:	player.current terdefinisi.
	F.S.	:	komponen ditulis sesuai dengan format yang diinginkan.
*/
{
	printf ("%s 		| LVL:%d HP:%d/%d MP:%d/%d STR:%d DEF:%d EXP:%d UpgradePoint: %d", player_current.name, player_current.LVL, player_HP_current, player_current.HP, player_MP_current,player_current.MP, player_current.STR, player_current.DEF, player_current.EXP, player_current.UP);
}

void player_get_medicine (int* HP, int* MP, int MaxHP, int MaxMP)
{
	*HP += 150;
	*MP += 50;

	if (*HP > MaxHP)
	{
		*HP = MaxHP;
	}

	if (*MP > MaxMP)
	{
		*MP = MaxMP;
	}
}

void  load_player_skill(TriTree *P, char* file_name)
{
	
	/*
	char bufferName[100];
	char bufferTaken[100];
	boolean taken_status;
	
	START(file_name);
	
	while (!EndKata)
	{
		string_copy (bufferName, CKata.TabKata);
		ADVKATA();

		string_copy (bufferTaken, CKata.TabKata);
		taken_status = atoi (bufferTaken);

		ADVKATA();
	}
	*/
	char* stringOfTree = (char *) malloc(250);
	int idx = 0;
	STARTKATA(file_name);
	string_copy (stringOfTree, CKata.TabKata);
	BuildTreeFromString (&*P, stringOfTree, &idx);
}

void player_choose_skill(char* s, STATS *player)
{
	//(God's_Blessing|0|2(Warrior's_Strength|0|3(Warcry|0|4()()())(Backstab|0|4()()())(Disarm|0|4()()()))()(Knight's_Fortitude|0|2(6th_Sense|0|4()()())(Riposte|0|4()()())()).
	TriTree P;

	P = SearchPB(skill_tree, s);

	if (P!= NULL && Level(P) <= (*player).LVL && !Taken(P))
	{
		if (string_compare(Skill(P),"God's_Blessing"))
		{
			(*player).HP+=100;
			(*player).MP+=20;
		} else if (string_compare(Skill(P),"Warrior's_Strength"))
		{
			(*player).STR += 10;
		} else if (string_compare(Skill(P),"Knight's_Fortitude"))
		{
			(*player).DEF += 10;
		}

		printf ("Anda berhasil mendapatkan skill %s!\n", Skill(P));
		Taken(P) = 1;
		(*player).UP -= 1;

	}else if (P == NULL)
	{
		printf ("Tidak ada skill dengan nama itu!\n");
	}else if (Level(P) > (*player).LVL)
	{
		printf ("Level anda tidak mencukupi. Kok bisa tahu??\n");
	}else if (Taken(P))
	{
		printf ("Anda sudah mengambil skill itu.\n");
	}
}
/////////////////////////////////	END		///////////////////////////////////////////////////////

/********************************	SEGALA TENTANG MUSUH		**********************************************/

void load_enemy (STATS *enemy_current, char* enemy_file) //MUNGKIN PERLU DITAMBAH PEMBACAAN AKSI MUSUH.
/*
	Membaca stats musuh dari file eksternal.
	
	I.S.	:	<enemy_file>.txt pasti ada.
				enemy_current sembarang.
	F.S.	:	komponen enemy_current terdefinisi semua.
*/
{
	char buffer[200];
	
	STARTKATA(enemy_file);
	
	string_copy ((*enemy_current).name, CKata.TabKata);
	
	ADVKATA();
	string_copy (buffer, CKata.TabKata);
	(*enemy_current).LVL = atoi (buffer);
	
	ADVKATA();
	string_copy (buffer, CKata.TabKata);
	(*enemy_current).HP = atoi (buffer);
	enemy_HP_current = (*enemy_current).HP;
	
	ADVKATA();
	string_copy (buffer, CKata.TabKata);
	(*enemy_current).MP = atoi (buffer);
	
	ADVKATA();
	string_copy (buffer, CKata.TabKata);
	(*enemy_current).STR = atoi (buffer);
	
	ADVKATA();
	string_copy (buffer, CKata.TabKata);
	(*enemy_current).DEF = atoi (buffer);
	
	ADVKATA();
	string_copy (buffer, CKata.TabKata);
	(*enemy_current).EXP = atoi (buffer);
}

char enemy_command_generate () //UNTUK BOSS BATTLE.
/*
	Mengambil angka random dari [0,2]. 0 = A, 1 = B, 2 = F. srand diambil dari waktu saat ini.
*/

{
	
	int r;
	
	do
	{
		r = rand();
	}while (r > 3);
	
	if (r == 0)
	{
		return 'A';
	}else if (r == 1)
	{
		return 'B';
	}else 
	{
		return 'F';
	}
	
}

/////////////////////////////////	END		///////////////////////////////////////////////////////

/********************************	SEGALA TENTANG PERTARUNGAN		**********************************************/
void battle_transition (char* map_name, int x, int y) //MUNGKIN PEMBACAAN AKSI FILE EKESTERNAL BISA DI SINI. Stack PERLU DIGANTI QUEUE.
/*
	Transisi ke layar pertarungan dari layar peta berdasarkan nama peta dan posisi musuh pada peta.
	
	I.S.	:	<map_name>.txt pasti ada. x dan y terdefinisi dalam matriks peta.
				battle_over = false.
				player_command_done = false.
				round = 1.
				
	F.S.	:	Setelah ditemukan musuh dengan "alamat" yang sesuai, status dan aksi musuh di load dan pertarungan 
				dimulai sampai pemain atau musuh mati atau ronde > 10 ; battle_over = true. 
				
				Tiap ronde terdiri atas 4 aksi. Aksi dari musuh ditampilkan, tetapi aksi urut genap disamarkan dengan
				'#'. Pemain kemudian bisa memasukkan 4 aksi untuk merespon aksi musuh. Setelah pemain memasukkan aksi,
				aksi musuh ditampilkan semua dan interaksi aksi dan kalkulasi damage (STR - DEF) ditampilkan. 
				
*/


{
	
	string_copy (mod, "mod");
	if (string_compare(map_name, "map_hubx.txt"))
	{
		//if (x == 10 && y == 7)
		{
			int r;
			do
			{
				r = rand();
			}while (r > 1);
			r = 0;

			Queue enemy_command_buffer;
			Stack enemy_command_stack;
			int round = 1;
			int action;
			char action_name;
			char buffer;
			char* file_name_buffer = (char *) malloc (100);
			count = 1;
			
			if (r == 1)
			{
				char file_name [] = "enemy_undead_knight";
			
				load_enemy (&enemy_current,"enemy_undead_knight.txt");
				string_copy (file_name_buffer, file_name);
				string_concat (file_name_buffer, "_command.txt");
				CreateEmpty (&enemy_command_stack);

				START(file_name_buffer);
				
				while (!EOP)
				{
					Push (&enemy_command_stack, CC);
					ADV();
					
				}
				battle_over = false;
				
				while (!battle_over && round <= 10)
				{
					CreateEmpty_Q (&enemy_command, 4);
					CreateEmpty_Q (&enemy_command_buffer, 4);
					printf ("Round %d", round);newLine;
					player_stats_tulis();newLine;
					printf ("%s 	| HP:%d/%d ", enemy_current.name, enemy_HP_current, enemy_current.HP);
					
					for (action = 0 ; action <= 3; action++)
					{
						Pop (&enemy_command_stack, &action_name);
						Add (&enemy_command, action_name);
						Add (&enemy_command_buffer, action_name);
					
					}
					
					for (action = 1; action <= 4; action++)
					{
						Del (&enemy_command_buffer, &action_name);
						
						if (action % 2 == 1)
						{
							printf ("%c ", action_name);
						}else 
						{
							printf ("# ");
						}
					}
					newLine;
					
					CreateEmpty_L (&player_command);
					player_command_done = false;
					while (!player_command_done)
					{
						action = 1;
						while (action <= 4)
						{
							printf ("Insert action: "); scanf("%c", &action_name);getchar();
							switch (action_name)
							{
								case 'A':	InsVLast (&player_command, action_name);
											action++;
										break;
								case 'B':	InsVLast (&player_command, action_name);
											action++;
										break;
								case 'F':	InsVLast (&player_command, action_name);
											action++;
										break;
								case 'E':	DelVLast (&player_command, &buffer);
											action--;
										break;
								default	: break;
							}				
						}
						
						printf ("All action: "); PrintInfo (player_command);newLine;
						
						do
						{
							printf ("Confirm action (y/n): "); scanf ("%c",&action_name); getchar();
						}
						while (action_name != 'y' && action_name != 'n' && action_name != 'm');
						
						if (action_name == 'y')
						{
							player_command_done = true;
						}else if (action_name == 'n')
						{
							while (!IsEmpty_L(player_command))
							{
								DelVLast (&player_command, &buffer);
							}
						}else //m
						{
							printf ("Tuliskan mod; jika salah, pertarungan dilanjutkan dengan normal!\n");
							printf ("Suicide\nWin\nEscape\n");
							scanf ("%s", mod);getchar();
							player_command_done = true;
						}
					}
					
					newLine;
					
					if (string_compare(mod, "Suicide"))
					{
						printf ("Suicide mod activated..."); newLine;
						player_HP_current = 0;
					}else if (string_compare (mod,"Win"))
					{
						printf ("Win mod activated..."); newLine;
						enemy_HP_current = 0;
					}else if (string_compare (mod, "Escape"))
					{
						printf ("Escape mod activated...");newLine;
						round = 10;
					}
					
					battle_interaction(player_command, enemy_command, &player_current, &enemy_current);
					
					if (enemy_HP_current <= 0 || player_HP_current <= 0)
					{
						battle_over = true;
					}

					round++;
					newLine;
				}
			} else if (r == 0)
			{
				
				char file_name [] = "enemy_skeleton_soldier";
			
				load_enemy (&enemy_current,"enemy_skeleton_soldier.txt");
				string_copy (file_name_buffer, file_name);
				string_concat (file_name_buffer, "_command.txt");
				
				START(file_name_buffer);
				while (!EOP)
				{
					Push (&enemy_command_stack, CC);
					ADV();
					
				}
				battle_over = false;
				
				while (!battle_over && round <= 10)
				{
					CreateEmpty_Q (&enemy_command, 4);
					CreateEmpty_Q (&enemy_command_buffer, 4);
					printf ("Round %d", round);newLine;
					player_stats_tulis();newLine;
					printf ("%s 	| HP:%d/%d ", enemy_current.name, enemy_HP_current, enemy_current.HP);
					
					for (action = 0 ; action <= 3; action++)
					{
						Pop (&enemy_command_stack, &action_name);
						Add (&enemy_command, action_name);
						Add (&enemy_command_buffer, action_name);
					
					}
					
					for (action = 1; action <= 4; action++)
					{
						Del (&enemy_command_buffer, &action_name);
						
						if (action % 2 == 1)
						{
							printf ("%c ", action_name);
						}else 
						{
							printf ("# ");
						}
					}
					newLine;
					
					
					CreateEmpty_L (&player_command);
					player_command_done = false;
					while (!player_command_done)
					{
						action = 1;
						while (action <= 4)
						{
							printf ("Insert action: "); scanf("%c", &action_name);getchar();
							switch (action_name)
							{
								case 'A':	InsVLast (&player_command, action_name);
											action++;
										break;
								case 'B':	InsVLast (&player_command, action_name);
											action++;
										break;
								case 'F':	InsVLast (&player_command, action_name);
											action++;
										break;
								case 'E':	DelVLast (&player_command, &buffer);
											action--;
										break;
								case 'S':	/*
											Jika ada skill
												-tampilkan skill
												-input pilihan
												-simpan pilihan
												
								*/
								default	: break;
							}				
						}
						
						printf ("All action: "); PrintInfo (player_command);newLine;
						
						do
						{
							printf ("Confirm action (y/n): "); scanf ("%c",&action_name);getchar();
						}
						while (action_name != 'y' && action_name != 'n' && action_name != 'm');
						
						if (action_name == 'y')
						{
							player_command_done = true;
						}else if (action_name == 'n')
						{
							while (!IsEmpty_L(player_command))
							{
								DelVLast (&player_command, &buffer);
							}
						}else //m
						{
							printf ("Tuliskan mod; jika salah, pertarungan dilanjutkan dengan normal!\n");
							printf ("Suicide\nWin\nEscape\n");
							scanf ("%s", mod);getchar();
							player_command_done = true;
						}
					}
					
					newLine;
					
					if (string_compare(mod, "Suicide"))
					{
						printf ("Suicide mod activated..."); newLine;
						player_HP_current = 0;
					}else if (string_compare (mod,"Win"))
					{
						printf ("Win mod activated..."); newLine;
						enemy_HP_current = 0;
					}else if (string_compare (mod, "Escape"))
					{
						printf ("Escape mod activated...");newLine;
						round = 10;
					}

					battle_interaction(player_command, enemy_command, &player_current, &enemy_current);
					
					if (enemy_HP_current <= 0 || player_HP_current <= 0)
					{
						battle_over = true;
					}

					
					
					round++;
					newLine;					
				}
			
			
			}else if (r == 10)
			{
				
				char file_name [] = "enemy_remnant_of_despair";
			
				load_enemy (&enemy_current,"enemy_remnant_of_despair.txt");
				
				battle_over = false;
				
				while (!battle_over && round <= 10)
				{
					CreateEmpty_Q (&enemy_command, 4);
					CreateEmpty_Q (&enemy_command_buffer, 4);
					printf ("Round %d", round);newLine;
					player_stats_tulis();newLine;
					printf ("%s 	| HP:%d/%d ", enemy_current.name, enemy_HP_current, enemy_current.HP);
					
					for (action = 0 ; action <= 3; action++)
					{
						action_name = enemy_command_generate();
						Add (&enemy_command, action_name);
						Add (&enemy_command_buffer, action_name);
					
					}
					
					for (action = 1; action <= 4; action++)
					{
						Del (&enemy_command_buffer, &action_name);
						
						if (action % 2 == 1)
						{
							printf ("%c ", action_name);
						}else 
						{
							printf ("# ");
						}
					}
					newLine;
					
					
					CreateEmpty_L (&player_command);
					player_command_done = false;
					while (!player_command_done)
					{
						action = 1;
						while (action <= 4)
						{
							printf ("Insert action: "); scanf("%c", &action_name);getchar();
							switch (action_name)
							{
								case 'A':	InsVLast (&player_command, action_name);
											action++;
										break;
								case 'B':	InsVLast (&player_command, action_name);
											action++;
										break;
								case 'F':	InsVLast (&player_command, action_name);
											action++;
										break;
								case 'E':	DelVLast (&player_command, &buffer);
											action--;
										break;
								default	: break;
							}				
						}
						
						printf ("All action: "); PrintInfo (player_command);newLine;
						
						do
						{
							printf ("Confirm action (y/n): "); scanf ("%c",&action_name);getchar();
						}
						while (action_name != 'y' && action_name != 'n' && action_name != 'm');
						
						if (action_name == 'y')
						{
							player_command_done = true;
						}else if (action_name == 'n')
						{
							while (!IsEmpty_L(player_command))
							{
								DelVLast (&player_command, &buffer);
							}
						}else //m
						{
							printf ("Tuliskan mod; jika salah, pertarungan dilanjutkan dengan normal!\n");
							printf ("Suicide\nWin\nEscape\n");
							scanf ("%s", mod);getchar();
							player_command_done = true;
						}
					}
					
					newLine;
					
					if (string_compare(mod, "Suicide"))
					{
						printf ("Suicide mod activated..."); newLine;
						player_HP_current = 0;
					}else if (string_compare (mod,"Win"))
					{
						printf ("Win mod activated..."); newLine;
						enemy_HP_current = 0;
					}else if (string_compare (mod, "Escape"))
					{
						printf ("Escape mod activated...");newLine;
						round = 10;
					}
					
					battle_interaction(player_command, enemy_command, &player_current, &enemy_current);
					
					if (enemy_HP_current <= 0 || player_HP_current <= 0)
					{
						battle_over = true;
					}

					
					
					round++;
					newLine;					
				}
			
			
			}
		}
	}
}

void battle_interaction(List player_command, Queue enemy_command, STATS *player_current, STATS *enemy_current)
/*
	I.S.	:	player_current dan enemy_current terdefinisi.
				player_command dan enemy_command terdefinisi.
	F.S.	:	Memasukkan aksi-aksi karakter ke array berukuran 4. Aksi dilakukan secara berurutan. Bisa berakhir
				di tengah prosedur jika salah satu karakter mati.
	
				Menampilkan pesan yang berisi aksi yang dilakukan pemain dan musuh, serta kalkulasi damage. Jika 
				damage < 0, maka akan dijadikan bernilai 0; DEF >= STR.
				
				Menampilkan pesan "Kamu mati" jika player_current.HP <= 0, player_dead = true, logged_in = false.
				Menampilkan pesan "Musuh mati" jika enemy_current.HP <= 0, enemy_dead = true.
				
				Daftar interaksi:
					A vs A || F vs F = pemain dan musuh mendapat damage,
					A vs B || B vs B = Tidak terjadi kalkulasi damage,
					A vs F = karakter beraksi F menerima damage,
					F vs B = karakter beraksi B menerima damage,
					
				
*/
{
	char enemy_command_buffer[4];
	char player_command_buffer[4];
	int i;
	
	for (i = 0; i <= 3; i++)
	{
		Del (&enemy_command, &enemy_command_buffer[i]);
	}
	
	for (i = 0; i <= 3; i++)
	{
		DelVFirst (&player_command, &player_command_buffer[i]);
	}
	
	player_stats_tulis();newLine;
	printf ("%s 	| HP:%d/%d ", (*enemy_current).name, enemy_HP_current,(*enemy_current).HP);
	for (i = 0; i <= 3; i++)
	{				
		printf ("%c ", enemy_command_buffer[i]);
	}
	newLine;
	
	player_dead = false;
	enemy_dead = false;
	
	i = 0;
	while (i <= 3 && !enemy_dead && !player_dead)
	{
		
			
		if (player_command_buffer[i] == 'A' && enemy_command_buffer[i] =='A')
		{
			printf ("%s attacked! You suffered %d damage!", (*enemy_current).name, abszero((*enemy_current).STR - (*player_current).DEF));newLine;
			printf ("%s also attacked! Enemy suffered %d damage!", (*player_current).name, abszero((*player_current).STR - (*enemy_current).DEF));newLine;
			player_HP_current -= abszero((*enemy_current).STR - (*player_current).DEF);
			enemy_HP_current -= abszero((*player_current).STR - (*enemy_current).DEF);;
		}else if (player_command_buffer[i] == 'F' && enemy_command_buffer[i] =='F')
		{
			printf ("%s flanked! You suffered %d damage!", (*enemy_current).name, abszero((*enemy_current).STR - (*player_current).DEF));newLine;
			printf ("%s also flanked! Enemy suffered %d damage!", (*player_current).name, abszero((*player_current).STR - (*enemy_current).DEF));newLine;
			player_HP_current -= abszero((*enemy_current).STR - (*player_current).DEF);
			enemy_HP_current -= abszero((*player_current).STR - (*enemy_current).DEF);
		}
		/////////////////////////////////////////////////////////////////////////
		else if (player_command_buffer[i] == 'A' && enemy_command_buffer[i] =='F')
		{
			printf ("%s tried to flank!", (*enemy_current).name);newLine;
			printf ("%s dodged and attacked! Enemy suffered %d damage!", (*player_current).name, abszero((*player_current).STR - (*enemy_current).DEF));newLine;
			enemy_HP_current -= abszero((*player_current).STR - (*enemy_current).DEF);
		}else if (player_command_buffer[i] == 'F' && enemy_command_buffer[i] =='A')
		{
			printf ("%s tried to flank!", (*player_current).name);newLine;
			printf ("%s dodged and attacked! You suffered %d damage!", (*enemy_current).name, abszero((*enemy_current).STR - (*player_current).DEF));newLine;
			player_HP_current -= abszero((*enemy_current).STR - (*player_current).DEF);
		}
		//////////////////////////////////////////////////////////////////////////
		else if (player_command_buffer[i] == 'B' && enemy_command_buffer[i] =='F')
		{
			printf ("%s tried to block!", (*player_current).name);newLine;
			printf ("But %s flanked! You suffered %d damage!", (*enemy_current).name, abszero((*enemy_current).STR - (*player_current).DEF));newLine;
			player_HP_current -= abszero((*enemy_current).STR - (*player_current).DEF);
		}else if (player_command_buffer[i] == 'F' && enemy_command_buffer[i] =='B')
		{
			printf ("%s tried to block!", (*enemy_current).name);newLine;
			printf ("But %s flanked! Enemy suffered %d damage!", (*player_current).name, abszero((*player_current).STR - (*enemy_current).DEF));newLine;
			enemy_HP_current -= abszero((*player_current).STR - (*enemy_current).DEF);
		}
		///////////////////////////////////////////////////////////////////////////
		else if (player_command_buffer[i] == 'A' && enemy_command_buffer[i] =='B')
		{
			printf ("%s tried to attack!", (*player_current).name);newLine;
			printf ("But %s blocked!", (*enemy_current).name);newLine;
		}else if (player_command_buffer[i] == 'B' && enemy_command_buffer[i] =='A')
		{
			printf ("%s tried to attack!", (*enemy_current).name);newLine;
			printf ("But %s blocked!", (*player_current).name);newLine;
		}
		//////////////////////////////////////////////////////////////////////////////
		else
		{
			printf ("%s tried to block!", (*enemy_current).name);newLine;
			printf ("%s also blocked!", (*player_current).name);newLine;
		}
		newLine;


		
		if (player_HP_current <= 0)
		{
			printf ("You are dead!");newLine;
			player_dead = true;
			logged_in = false;
		}else if (enemy_HP_current <= 0)
		{
			printf ("Enemy slain! returning to map...");newLine;
			enemy_dead = true;
			Get_Exp(player_current, (*enemy_current).EXP);
			lvl_up_req = lvl_up((*player_current).LVL);
			printf("%d\n", lvl_up_req);
			while((player_current->EXP)>=lvl_up_req){
				printf("Level Up! Stats Upgraded:\n");
				printf ("1 upgrade point received!\n");
				(*player_current).EXP-= lvl_up_req;
				(*player_current).LVL++;
				(*player_current).HP+=25;
				(*player_current).MP+=5;
				(*player_current).STR+=3;
				(*player_current).DEF+=3;
				(*player_current).UP+=1;

				player_stats_tulis();
				pressEnterPrompt;
				getchar();
			}
			pressEnterPrompt;
			getchar();
		}else
		{
			i++;
		}
	}
}

void Get_Exp(STATS *player_current, int enemy_exp)
{
	(*player_current).EXP+=enemy_exp;
}

int lvl_up(int current_level)
{
	return (35 + (((current_level))*15));
}
/////////////////////////////////	END		///////////////////////////////////////////////////////

/********************************	LAIN-LAIN		**********************************************/
int abszero(int x)
/*
	Fungsi untu memastika nilai (STR - DEF) selalu >= 0; Jika DEF > STR mengembalikan 0.
*/
{
	if (x <= 0)
	{
		return 1;
	}else
	{
		return x;
	}
}


/////////////////////////////////	END		///////////////////////////////////////////////////////
