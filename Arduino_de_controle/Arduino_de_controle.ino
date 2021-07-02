/* ---------------------------------------------------------------------------------------------------------- */
/*  Inclui as bibliotecas que serão utilizadas no código.                                                     */
#include <LiquidCrystal.h>
/* ---------------------------------------------------------------------------------------------------------- */
/*  Configurações de sistema.                                                                                 */
//  ~ Configurações de horário do relógio.
#define START_HOUR 20
#define START_MINUTE 25
#define START_SECOND 10
//  ~ Tempo para atualização do relógio.
#define CLOCK_REFRESH_DELAY 1000
//  ~ Horario de acionamento automático das luzes da sala.
#define LIVING_ROOM_LIGHTS_TIME_TO_ON 20
//  ~ Horario de desligamento automático das luzes da sala.
#define LIVING_ROOM_LIGHTS_TIME_TO_OFF 22
//  ~ Intervalo no qual as luzes da sala ficam ligadas após o acionamento do sensor de presença.
#define LIVING_ROOM_LIGHTS_PIR_INTERVAL 5000
//  ~ Intervalo no qual o menu pode ficar parado.
#define MENU_ACCEPT_SLEEP_INTERVAL 5000
//  ~ Limite do menu.
#define MENU_LIMIT 2
//  ~ Temperatura inicial.
#define TEMPERATURE 24
//  ~ Estado inicial do alarme.
#define ALARM false
/* ---------------------------------------------------------------------------------------------------------- */
/*  Definição de referências de uso para as portas do arduino.                                                */
//  ~ Função UP do menu.
#define MENU_UP 2
//  ~ Função DOWN do menu.
#define MENU_DOWN 3
//  ~ Botão de controle do menu.
#define MENU_BUTTON 4
//  ~ Luz de fundo do display.
#define LCD_BACKGROUND_LED 7
//  ~ Porta que define se o horário deve ser exibido em formato a.m./p.m. ou não.
#define HOUR_AMERICAN_MODE 5
//  ~ Porta do interruptor que troca a luz da sala entre automática e manual.
#define LIVING_ROOM_AUTO_LIGHTS 6
//  ~ Porta de luz de fundo do LCD.
#define LCD_BACKGROUND 7
//  ~ Portas de informação do LCD.
#define LCD_0 11
#define LCD_1 10
#define LCD_2 9
#define LCD_3 8
//  ~ Porta de ativação do LCD.
#define LCD_ACTIVE 12
//  ~ Porta de seção de registro do LCD.
#define LCD_RECORDS_SECTION 13
//  ~ Porta responsável pelo interruptor manual de luz da sala.
#define LIVING_ROOM_LIGHTS_INT A0
//  ~ Porta responsável pelo sensor de presença da sala.
#define LIVING_ROOM_PIR_SENSOR A1
//  ~ Luzes da sala.
#define LIVING_ROOM_LEDS A2
//  ~ Porta responsável pelo sensor de luz ambiente.
#define LIVING_ROOM_LIGHT_SENSOR A3
/* ---------------------------------------------------------------------------------------------------------- */
/*  Variáveis do sistema de relógio.                                                                          */
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
//  ~ Informa se as luzes estão acesas ou apagadas. Como isso depende das relações envolvidas no loop(), 
//  o sistema inicaliza como apagadas.
bool livingRoom_lights;
//  ~ Informa se as luzes foram ligadas com o PIR ou não.
bool livingRoom_lights_on_with_PIR;
//  ~ Sinal do sensor de luz ambiente.
int livingRoom_lightSensor_signal;
//  ~ Instante no qual a luz foi ligada.
unsigned long livingRoom_time_lights_on;
/* ---------------------------------------------------------------------------------------------------------- */
/*  Variáveis do sistema do LCD.                                                                              */
//  ~ Elemento que representa o LCD no código e responsável por seu funcionamento.
LiquidCrystal lcd (LCD_RECORDS_SECTION, LCD_ACTIVE, LCD_0, LCD_1, LCD_2, LCD_3);
//  ~ Buffers das linhas do arduino.
String LCDRow[2];
/* ---------------------------------------------------------------------------------------------------------- */
/*  Variáveis do menu.                                                                                        */
//  ~ Variável que salva a tela do menu.
//  A tela '0' é a padrão, exibindo uma informação em branco.
//  A tela '1' permite a alteração da temperatura.
//  A tela '2' permite a alteração do alarme.
//  A tela '3' permite a alteração do horário do despertador.
//  A tela '4' permite a alteração dos minutos do despertador (variando de 15 em 15).
//  A tela '5' permite a alteração do horário do relógio.
//  A tela '6' permite a alteração dos minutos do relógio.
//  A tela '7' permite a alteração dos segundos do relógio.
//    Após a tela apagar ele retorna automáticamente para a '0'
int menu_screen;
//  ~ Registra o instante do millis() em que a tela foi ligada (botão menu foi pressionado).
unsigned long active_lcd_menu;
//  ~ Controle de temperatura.
float temperature;
//  ~ Alarme esta ligado ou desligado?
bool alarm_isActive;
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
/*  Faz o envio de informação.                                                                                */
void Send(int code, int message)
{
  //  ~ Buffer de envio.
  unsigned long _buffer;
  //  ~ Adiciona ao buffer a mensagem.
  _buffer = (unsigned long) (message);
  //  ~ Adiciona a esquerda da mensagem o código.
  _buffer += (code * 100000UL);
  //  ~ Envia a mensagem.
  Serial.println(_buffer);
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
  pinMode(LCD_BACKGROUND, INPUT);
  pinMode(LIVING_ROOM_LIGHTS_INT, INPUT);
  pinMode(LIVING_ROOM_PIR_SENSOR, INPUT);
  pinMode(MENU_BUTTON, INPUT);
  pinMode(MENU_UP, INPUT);
  pinMode(MENU_DOWN, INPUT);

  //  ~ Porta de saída de dados ('OUTPUT') do arduino.
  pinMode(LIVING_ROOM_LEDS, OUTPUT);
  pinMode(LCD_BACKGROUND_LED, OUTPUT);
  
  //  ~ Inicializa o valor das variáveis.
  //  Ele armazena na variável 'livingRoom_auto_mode' o inverso do respectivo interruptor, para garantir o acionamento no primeiro loop.
  livingRoom_auto_mode = !((bool) digitalRead(LIVING_ROOM_AUTO_LIGHTS));
  //  Armazena o formato de horário na variável respectiva.
  american_clock_format = (bool) digitalRead(HOUR_AMERICAN_MODE);
  //  Torna a variável da sala como desligada por enquanto.
  livingRoom_lights = false;  
  //  Coloca o uso do sensor PIR como falso, momentaneamente.
  livingRoom_lights_on_with_PIR = false;
  //  Inicializa o sensor de luz ambiente.
  livingRoom_lightSensor_signal = -1;
  //  Momento no qual as luzes foram acesas (define como 0).
  livingRoom_time_lights_on = 0;
  //  Define como padrão a tela 0.
  menu_screen = 0;
  //  Zera o horário de abertura do menu.
  active_lcd_menu = 0;
  //  Determina a temperatura inicial.
  temperature = TEMPERATURE;
  //  Determina o padrão do alarme.
  alarm_isActive = !ALARM;
  //  Inicializa os horários do relógio.
  hour = START_HOUR;
  minute = START_MINUTE;
  second = START_SECOND;
  //  ~ Inicializa o relógio.
  ClockRefresh();
  //  ~ Efetiva a atuação de 'livingRoom_lights'.
  digitalWrite(LIVING_ROOM_LEDS, (int) livingRoom_lights);

  //  ~ Inicializa com a tela inicial.
  LCDWrite("~~ AUTO-CASA! ~~", 0);
}
/* ---------------------------------------------------------------------------------------------------------- */
/*  Loop principal do sistema.                                                                                */
void loop()
{
  //  ~ Declara e inicializa as variaveis comparativas de estado.
  bool _livingRoom_auto_mode = (bool) digitalRead(LIVING_ROOM_AUTO_LIGHTS);
  bool _livingRoom_lights = false;
  int _livingRoom_lightSensor_signal = analogRead(LIVING_ROOM_LIGHT_SENSOR);
  int _menu_screen = menu_screen;

  float _temperature = temperature;
  bool _alarm_isActive = alarm_isActive;
  //  ~ Variáveis de estado locais.
  bool livingRoom_light_low;

  //  ~ Captura e atualiza os valores de estado nas variáveis.
  //  Formato de hora.
  american_clock_format = (bool) digitalRead(HOUR_AMERICAN_MODE);

  //  ~ Verifica atualização no sensor de luz ambiente.
  if (livingRoom_lightSensor_signal != _livingRoom_lightSensor_signal)
  {
    //  ~ Altera o valor.
    livingRoom_lightSensor_signal = _livingRoom_lightSensor_signal;
    //  ~ Calcula o percentual de luminosidade.
    int light_percent = map(_livingRoom_lightSensor_signal, 550, 1023, 100, 0);
    //  ~ Se a luminosidade for inferior a 40%, informa em 'livingRoom_light_low' (true), se não, (false).
    if (light_percent < 40) livingRoom_light_low = true; else livingRoom_light_low = false;
  }

  //  ~ Verifica se as luzes estão em modo automático ou manual (para evitar sobrecarda de chamados ele faz uso da
  //  variável local na validação.
  if (_livingRoom_auto_mode)
  {
    //  ~ No modo automático, a luz é ligada de um horário fixo regulado por enquanto nos #define. Futuramente, é
    //  possível alterar para um sistema de configuração via menu do LED com controle UP/DOWN.
    if ((hour >= LIVING_ROOM_LIGHTS_TIME_TO_ON) and (hour <= LIVING_ROOM_LIGHTS_TIME_TO_OFF)) { _livingRoom_lights = true; livingRoom_lights_on_with_PIR = false; }
    //  ~ Caso a luz não seja acesa devido ao horário, ela pode ser acesa por conta da luminosidade baixa combinada
    //  com o sensor PIR.
    else if ((livingRoom_light_low) and (digitalRead(LIVING_ROOM_PIR_SENSOR))) { livingRoom_lights_on_with_PIR = true; livingRoom_time_lights_on = millis(); }
    //  ~ Por fim, caso nenhuma das opções realize o acendimento da luz, ela é desligada.
    else _livingRoom_lights = false;
    //  ~ Se a luz tiver sido acesa pelo PIR em algum momento, faz a atualização.
    if (livingRoom_lights_on_with_PIR) _livingRoom_lights = true;
    //  ~ Se a luz tiver sido acesa pelo sensor PIR, ela deve ficar acesa por um tempo limitado, quem controla isso é o millis().
    if ((livingRoom_lights_on_with_PIR) and (millis() >= (livingRoom_time_lights_on + LIVING_ROOM_LIGHTS_PIR_INTERVAL))) { _livingRoom_lights = false; livingRoom_lights_on_with_PIR = false; }
  }
  else
  {
    //  ~ Basea-se no interruptor de ligado/desligado.
    _livingRoom_lights = (bool) digitalRead(LIVING_ROOM_LIGHTS_INT);
  }
  
  //  ~ Verifica atualizações de estado.
  //  Interruptor de modo manual ou automático das luzes da sala.
  if (livingRoom_auto_mode != _livingRoom_auto_mode) livingRoom_auto_mode = _livingRoom_auto_mode;
  //  Luzes da sala.
  if (livingRoom_lights != _livingRoom_lights) 
  {
    //  ~ Informa a alteração.
    livingRoom_lights = _livingRoom_lights;
    //  ~ Se for para acender, acende. Se não, apaga.
    digitalWrite(LIVING_ROOM_LEDS, (int) _livingRoom_lights);
  }

  //  ~ Atualiza o horário do relógio.
  if(millis() >= (lastClockRefresh + CLOCK_REFRESH_DELAY)) ClockRefresh();

  //  ~ Verifica se o botão do menu foi pressionado.
  if ((bool) digitalRead(MENU_BUTTON))
  {
    //  ~ Incrementa o menu.
    _menu_screen++;
    //  ~ Se o incremento ultrapassar 7, retorna para 1.
    if (_menu_screen > MENU_LIMIT) _menu_screen = 1;
    //  ~ Por fim, se o contador de inicialização do menu for 0, coloca a tela em '0' e inicializa.
    if (active_lcd_menu == 0) digitalWrite(LCD_BACKGROUND_LED, HIGH);
    //  ~ Inicia o contador.
    active_lcd_menu = millis();
    //  ~ Aguarda o término do pressionamento do botão.
    while (digitalRead(MENU_BUTTON)) ;
  }

  //  ~ Verifica se o menu está muito tempo parado.
  if (millis() >= (active_lcd_menu + MENU_ACCEPT_SLEEP_INTERVAL))
  {
    //  ~ Joga a tela em '0'.
    _menu_screen = 0;
    //  ~ Zera o tempo de ativação.
    active_lcd_menu = 0;
    //  ~ Desliga a luz de fungo.
    digitalWrite(LCD_BACKGROUND_LED, LOW);
  }

  //  ~ Verifica se MENU_UP foi pressionado.
  if (digitalRead(MENU_UP))
  {
    //  ~ Verifica o sentido do botão, basenado-se no menu.
    switch(_menu_screen)
    {
      //  ~ Controlador de temperatura.
      case 1:
        temperature += 2;
      break;
      //  ~ Alarme.
      case 2:
        alarm_isActive = true;
      break;
    }
    //  ~ Força uma atualização.
    menu_screen = -1;
    //  ~ Aguarda.
    while (digitalRead(MENU_UP)) ;
  }

  //  ~ Verifica se MENU_DOWN foi pressionado.
  if (digitalRead(MENU_DOWN))
  {
    //  ~ Verifica o sentido do botão, basenado-se no menu.
    switch(_menu_screen)
    {
      //  ~ Controlador de temperatura.
      case 1:
        temperature -= 2;
      break;
      //  ~ Alarme.
      case 2:
        alarm_isActive = false;
      break;
    }
    //  ~ Força uma atualização.
    menu_screen = -1;
    //  ~ Aguarda.
    while (digitalRead(MENU_DOWN)) ;
  }

  //  ~ Verifica alteração no menu.
  if (_menu_screen != menu_screen)
  {
    //  ~ Efetiva a atualização.
    menu_screen = _menu_screen;

    switch (_menu_screen)
    {
      // ~ Menu de abertura.
      case 0:
        LCDWrite("~~ AUTO-CASA! ~~", 0);
      break;
      //  ~ Controlador de temperatura.
      case 1:
        LCDWrite(String("T: " + String((int) temperature) + "." + String(((int) (temperature * 10.0)) % 10) + String((char) B10110000) + "C"), 0);
      break;
      //  ~ Alarme.
      case 2:
        String alarm_status = alarm_isActive ? "ON" : "OFF";
        LCDWrite(String("Alarme: " + alarm_status), 0);
      break;
    }
  }

  //  ~ Atualiza o arduino auxiliar.
  if (_temperature != temperature) { Send(1, temperature); }
  if (_alarm_isActive != alarm_isActive) { Send(2, _alarm_isActive); }
}
