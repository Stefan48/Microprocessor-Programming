#include <LiquidCrystal.h>

// initialize the LCD library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

#define MAX_CNT 51
// button pins
#define BT_LEFT 10
#define BT_RIGHT 9
#define BT_ENTER 8
#define BUZZER 6
// game modes
#define SINGLE 1
#define MULTI 2
// piles' positions
#define LEFT 0
#define MID 1
#define RIGHT 2

int mode, turn;
bool end_turn, end_game, mode_select, buzz; 
int pile_cnt[3];
int pile_pos[3];
int position, i;
int cnt, index;
double dcnt;

void setup()
{
  // set up the LCD's number of columns and rows
  lcd.begin(16, 2);
  // set pin modes for buttons and buzzer
  pinMode(BT_LEFT, INPUT_PULLUP);
  pinMode(BT_RIGHT, INPUT_PULLUP);
  pinMode(BT_ENTER, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  // initialize game variables
  mode = 0;
  turn = 0;
  end_turn = false;
  end_game = false;
  mode_select = true;
  buzz = false;
  // generate piles
  randomSeed(analogRead(6));
  for (i = LEFT; i <= RIGHT; ++i)
  {
    pile_cnt[i] = random(1, MAX_CNT);
  }
  pile_pos[0] = 2;
  pile_pos[1] = 7;
  pile_pos[2] = 12;
  position = MID;
  // enable mode select display
  lcd.setCursor(0, 0);
  lcd.print("1: single player");
  lcd.setCursor(0, 1);
  lcd.print("2: multiplayer");
}

void loop()
{
  if (end_game)
  {
    goto end_turn;
  }
  
  if (mode_select)
  {
    if (!digitalRead(BT_LEFT))
    {
      mode = SINGLE;
    }
    else if (!digitalRead(BT_RIGHT))
    {
      mode = MULTI;
    }
    if (mode)
    {
      mode_select = false;
      turn = random(1, 3);
      lcd.setCursor(0, 0);
      if (mode == SINGLE)
      {
        if (turn == 1)
        {
          lcd.print("Player 1 starts ");
        }
        else
        {
          lcd.print("Computer starts ");
        }
      }
      else
      {
        lcd.print("Player " + (String)turn + " starts ");
      }
      lcd.setCursor(0, 1);
      lcd.print("                ");
      delay(1500);
      lcd.setCursor(0, 0);
      lcd.print("                ");
      // print size of each pile
      for (i = LEFT; i <= RIGHT; ++i)
      {
        lcd.setCursor(pile_pos[i], 0);
        if (pile_cnt[i] < 10)
        {
          lcd.print(" " + (String)pile_cnt[i]);
        }
        else
        {
          lcd.print(pile_cnt[i]);
        }
      }
    }
    goto loop_end;
  }
  
  if (mode == SINGLE && turn == 2)
  {
    // compute best move
    index = 0;
    cnt = 0;
    int large_piles = 0;
    for (i = LEFT; i <= RIGHT; ++i)
    {
      if (pile_cnt[i] > 1)
      {
        large_piles++;
      }
    }
    bool near_end = (large_piles <= 1);
    if (near_end)
    {
      int moves = 0;
      int cnt_max = 0, index_cnt_max = 0;
      for (i = LEFT; i <= RIGHT; ++i)
      {
        if (pile_cnt[i] > 0)
        {
          moves++;
          if (pile_cnt[i] > cnt_max)
          {
            cnt_max = pile_cnt[i];
            index_cnt_max = i;
          }
        }
      }
      int odd = (moves % 2 == 1);
      index = index_cnt_max;
      if (cnt_max == 1 && odd)
      {
        cnt = 1;
      }
      else
      {
        cnt = cnt_max - odd;
      }
    }
    else
    {
      // not near end
      int nim_sum = pile_cnt[LEFT] ^ pile_cnt[MID] ^ pile_cnt[RIGHT];
      if (nim_sum == 0)
      {
        // random move
        do
        {
          index = random(LEFT, RIGHT + 1);
        } while (pile_cnt[index] == 0);
        cnt = random(1, pile_cnt[index] + 1);
      }
      else
      {
        int new_cnt;
        for (i = LEFT; i <= RIGHT; ++i)
        {
          new_cnt = pile_cnt[i] ^ nim_sum;
          if (new_cnt < pile_cnt[i])
          {
            index = i;
            cnt = pile_cnt[i] - new_cnt;
            break;
          }
        }
      }
    }
    // apply move
    pile_cnt[index] -= cnt;
    lcd.setCursor(pile_pos[index], 1);
    if (cnt < 10)
    {
      lcd.print(" " + (String)cnt);
    }
    else
    {
      lcd.print(cnt);
    }
    delay(2000);
    lcd.setCursor(0, 1);
    lcd.print("                ");
    
    // check end game
    if (pile_cnt[LEFT] == 0 && pile_cnt[MID] == 0 && pile_cnt[RIGHT] == 0)
    {
      end_game = true;
    }
    else
    {
      turn = 1;
    }
    
    goto loop_end;
  }

  // read input
  if (!digitalRead(BT_LEFT))
  {
    if (position > 0)
    {
      position--;
    }
  }
  else if (!digitalRead(BT_RIGHT))
  {
    if (position < 2)
    {
      position++;
    }
  }
  
  // compute amount to take
  cnt = analogRead(A0);
  cnt = map(cnt, 0, 1020, 0, pile_cnt[position]);
  if (cnt == 0 && pile_cnt[position] > 0)
  {
    cnt = 1;
  }
  if(!digitalRead(BT_ENTER))
  {
    if (cnt == 0)
    {
      lcd.setCursor(2, 1);
      lcd.print("Invalid move");
      delay(1000);
      lcd.setCursor(0, 1);
      lcd.print("                ");
    }
    else
    {
      pile_cnt[position] -= cnt;
      if (pile_cnt[0] == 0 && pile_cnt[1] == 0 && pile_cnt[2] == 0)
      {
        end_game = true;
        buzz = true;
      }
      cnt = 0;
      turn = 3 - turn;
      end_turn = true;
    }
  }
  // print data on LCD
  // print size of each pile
  for (i = LEFT; i <= RIGHT; ++i)
  {
    lcd.setCursor(pile_pos[i], 0);
    if (pile_cnt[i] < 10)
    {
      lcd.print(" " + (String)pile_cnt[i]);
    }
    else
    {
      lcd.print(pile_cnt[i]);
    }
  }
  
end_turn:
  // print amount to take
  // or turn message if end of turn
  if (end_turn)
  {
    if (end_game)
    {
      lcd.setCursor(0, 1);
      if (mode == SINGLE && turn == 2)
      {
        lcd.print("Computer wins!  ");
      }
      else
      {
        lcd.print("Player " + (String)turn + " wins! ");
      }
      if (buzz)
      {
        buzz = false;
        tone(BUZZER, 600);
        delay(2000);
        noTone(BUZZER);
      }
      delay(5000);
    }
    else
    {
      end_turn = false;
      lcd.setCursor(0, 1);
      if (mode == SINGLE && turn == 2)
      {
        lcd.print("Computer's turn ");
      }
      else
      {
        lcd.print("Player " + (String)turn + "'s turn ");
      }
      delay(750);
      lcd.setCursor(0, 1);
      lcd.print("                ");
      position = MID;
    }
  }
  else
  {
    for (i = LEFT; i <= RIGHT; ++i)
    {
      lcd.setCursor(pile_pos[i], 1);
      if (position == i)
      {
        if (cnt < 10)
        {
          lcd.print(" " + (String)cnt);
        }
        else
        {
          lcd.print(cnt);
        }
      }
      else
      {
        lcd.print("  ");
      }
    }
  }
  
loop_end:
  delay(250);
}
