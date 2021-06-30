/* ---------------------------------------------------------------------------------------------------------- */
/*  Inclui as bibliotecas que serão utilizadas no código.                                                     */
#include <LiquidCrystal.h>
/* ---------------------------------------------------------------------------------------------------------- */
/*  Configurações de sistema.                                                                                 */
//  ~ Configurações de horário do relógio.
#define START_HOUR 19
#define START_MINUTE 25
#define START_SECOND 10
//  ~ Tempo para atualização do relógio.
#define CLOCK_REFRESH_DELAY 1000
/* ---------------------------------------------------------------------------------------------------------- */
/*  Definição de referências de uso para as portas do arduino.                                                */
//  ~ Porta que define se o horário deve ser exibido em formato a.m./p.m. ou não.
#define HOUR_AMERICAN_MODE 6
//  ~ Porta do interruptor que troca a luz da sala entre automática e manual.
#define LIVING_ROOM_AUTO_LIGHTS 7
//  ~ Portas de informação do LCD.
#define LCD_0 11
#define LCD_1 10
#define LCD_2 9
#define LCD_3 8
//  ~ Porta de ativação do LCD.
#define LCD_ACTIVE 12
//  ~ Porta de seção de registro do LCD.
#define LCD_RECORDS_SECTION 13
/* ---------------------------------------------------------------------------------------------------------- */
/*  Variáveis do sistema de luz da sala.                                                                      */
//  ~ Respectivamente, hora, minuto e segundo exibidos no relógio.
int hour;
int minute;
int second;
//  ~ Armazena o valor da última atualização do relógio para observar o avanço de millis().
unsigned long lastClockRefresh;
//  ~ Se o horario deve ou não ser exibido no formato americano.
bool american_clock_format;
/* ---------------------------------------------------------------------------------------------------------- */
/*  Variáveis do sistema de luz da sala.                                                                      */
//  ~ Informa se a luz da sala está em modo automático ou não.
bool livingRoom_auto_mode;
/* ---------------------------------------------------------------------------------------------------------- */
/*  Variáveis do sistema do LCD.                                                                              */
//  ~ Elemento que representa o LCD no código e responsável por seu funcionamento.
LiquidCrystal lcd (LCD_RECORDS_SECTION, LCD_ACTIVE, LCD_0, LCD_1, LCD_2, LCD_3);
//  ~ Buffers das linhas do arduino.
String LCDRow[2];
/* ---------------------------------------------------------------------------------------------------------- */
/*  Função do arduino.                                                                                        */
//  ~ Existem duas versões da função de escrita.
//  Nessa versão, o arduino limpara uma linha dele e substitui ela pela mensagem informada pelo usuário.
//      'message': mensagem a ser inserida no LCD.
//      'row': linha na qual a mensagem vai ser escrita. '0' para a linha 1 e '1' para a linha 2.
void LCDWrite(String message, int row)
{
  //  ~ Se a mensagem for igual ao buffer, ignora.
  if (message == LCDRow[row]) return;
  //  ~ Se a mensagem for muito grande, ignora.
  if (message.length() > 16) return;
  //  ~ Se a linha for inválida, ignora.
  if ((row != 0) and (row != 1)) return;
  //  ~ Limpa a linha.
  lcd.setCursor(0, row);
  lcd.print("                ");
  //  ~ Substitui o buffer.
  LCDRow[row] = message;
  //  ~ Imprime a mensagem.
  lcd.setCursor(0, row);
  lcd.print(LCDRow[row]);
}
//  Nesta, por outro lado, o arduino apaga parte da mensagem, substituindo pela informada.
//      'message': mensagem a ser inserida no LCD.
//      'row': linha na qual a mensagem vai ser escrita.
//      'colum': coluna a partir da qual a mensagem vai ser inserida.
void LCDWrite(String message, int row, int colum)
{
  //  ~ Se a mensagem for muito grande, ignora.
  if (message.length() > (16 - (colum + 1))) return;
  //  ~ Se a linha for inválida, ignora.
  if ((row != 0) and (row != 1)) return;
  //  ~ Se a coluna for inválida, ignora.
  if ((colum < 0) and (colum > 15)) return;
  //  ~ Limpa a linha.
  lcd.setCursor(colum, row);
  lcd.print("                ");
  //  ~ Converte o buffer em uma array de caracteres.
  char _buffer[16];
  LCDRow[row].toCharArray(_buffer, 16);
  //  ~ Converte, então, a mensagem em uma cadeia de caracteres.
  char _msgBuffer[16];
  message.toCharArray(_msgBuffer, 16);
  //  ~ Realiza a substituição dos elementos nas devidas posições.
  for (int i = colum; i < 16; i++)
  {
    _buffer[i] = _msgBuffer[i - colum];
  }
  //  ~ Salva o novo buffer.
  LCDRow[row] = String(_buffer);
  //  ~ Imprime a mensagem.
  lcd.setCursor(0, row);
  lcd.print(LCDRow[row]);
}
/* ---------------------------------------------------------------------------------------------------------- */
/*  Realiza a passagem e o cálculo do tempo no relógio.                                                       */
void ClockRefresh()
{
  //  ~ Incrementa o valor de segundos em 1.
  second++;
  //  ~ Se o valor de segundos for igual (ou superior) a 60, reseta e incrementa os minutos.
  if(second >= 60) { second = 0; minute++; }
  //  ~ Se o valor de minutor for igual (ou superior) a 60, reseta e incrementa as horas.
  if (minute >= 60) { minute = 0; hour++; }
  //  ~ Se o valor de horas for igual (ou superior) a 24, reseta as horas.
  if (hour >= 24) hour = 0;
  // Declara a string que servirá de buffer para construção.
  String _buffer = "";
  //  ~ Constroí o formato.
  if (american_clock_format)
  {
    //  Informa se o horário é superior a 12h.
    bool pm = false;
    //  Armazena o horario temporariamente.
    int _hour;
    if (hour > 12) { pm = true; _hour = hour - 12; }
    //  Insere as horas no buffer.
    if (_hour < 10) _buffer = "0";
    _buffer = String(_buffer + String(_hour) + ":");
    //  Insere os minutos.
    if (minute < 10) _buffer = String(_buffer + "0");
    _buffer = String(_buffer + String(minute) + ":");
    //  Insere os segundos.
    if (second < 10) _buffer = String(_buffer + "0");
    _buffer = String(_buffer + String(second));
    //  Insere o "a.m." ou "p.m."
    if (pm) _buffer = String(_buffer + " p.m."); else _buffer = String(_buffer + " a.m.");
  }
  else
  {
    //  Insere as horas no buffer.
    if (hour < 10) _buffer = "0";
    _buffer = String(_buffer + String(hour) + ":");
    //  Insere os minutos.
    if (minute < 10) _buffer = String(_buffer + "0");
    _buffer = String(_buffer + String(minute) + ":");
    //  Insere os segundos.
    if (second < 10) _buffer = String(_buffer + "0");
    _buffer = String(_buffer + String(second));
  }
  //  ~ Imprime o horário.
  LCDWrite(_buffer, 1);
  //  ~ Atualiza o tempo da última atualização.
  lastClockRefresh = millis();
}
/* ---------------------------------------------------------------------------------------------------------- */
/*  Função de entrada do programa.                                                                            */
void setup()
{
  //  ~ Inicializa o Serial.
  Serial.begin(9600);
  while (!Serial);
  
  //  ~ Liga o arduino com configuração 16x2 (16 colunas em 2 linhas).
  lcd.begin(16, 2);

  //  ~ Portas de entrada de dados ('INPUT') do arduino.
  pinMode(LIVING_ROOM_AUTO_LIGHTS, INPUT);
  pinMode(HOUR_AMERICAN_MODE, INPUT);

  //  ~ Inicializa o valor das variáveis.
  //  Ele armazena na variável 'livingRoom_auto_mode' o inverso do respectivo interruptor, para garantir o acionamento no primeiro loop.
  livingRoom_auto_mode = !((bool) digitalRead(LIVING_ROOM_AUTO_LIGHTS));
  //  Armazena o formato de horário na variável respectiva.
  american_clock_format = (bool) digitalRead(HOUR_AMERICAN_MODE);
  
  //  ~ Inicializa os horários do relógio.
  hour = START_HOUR;
  minute = START_MINUTE;
  second = START_SECOND;
  //  ~ Inicializa o relógio.
  ClockRefresh();
}
/* ---------------------------------------------------------------------------------------------------------- */
/*  Loop principal do sistema.                                                                                */
void loop()
{
  //  ~ Declara e inicializa as variaveis comparativas de estado.
  bool _livingRoom_auto_mode = (bool) digitalRead(LIVING_ROOM_AUTO_LIGHTS);

  //  ~ Captura e atualiza os valores de estado nas variáveis.
  //  Formato de hora.
  american_clock_format = (bool) digitalRead(HOUR_AMERICAN_MODE);

  //  ~ Verifica atualizações de estado.
  //  Interruptor de modo manual ou automático das luzes da sala.
  if (livingRoom_auto_mode != _livingRoom_auto_mode) 
  {
    //  ~ Atualiza o estado do interruptor.
    livingRoom_auto_mode = _livingRoom_auto_mode;

    if (livingRoom_auto_mode) LCDWrite("Luz ligada", 0); else LCDWrite("Luz desligada", 0);
  }

  //  ~ Atualiza o horário do relógio.
  if(millis() >= (lastClockRefresh + CLOCK_REFRESH_DELAY)) ClockRefresh();
  delay(200);
}