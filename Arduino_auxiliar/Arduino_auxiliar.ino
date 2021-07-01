/* ---------------------------------------------------------------------------------------------------------- */
/*  Configurações de sistema.                                                                                 */
//  ~ Define o limiar de ruído do sensor de gás.
#define GAS_SENSOR_LIMIT 12
//  ~ Define o limiar de acionamento dos motores de contenção.
#define GAS_ENGINE_LIMIT 25
//  ~ Intervalo de funcionamento do motor para fechar o duto de gás.
#define GAS_ENGINE_TIME_INTERVAL 700
//  ~ Maior intervalo de tempo entre as notas do alerta de gás.
#define GAS_ALERT_MAX_TIME 4000
//  ~ Frequencia da nota do alerta de gás.
#define GAS_ALERT_FREQUENCY 392
/* ---------------------------------------------------------------------------------------------------------- */
/*  Definição de referências de uso para as portas do arduino.                                                */
//  ~ Alerta de vazamento de gás.
#define GAS_ALERT 11
//  ~ Porta do motor de fechamento da válvula de gás.
#define GAS_FORCE_CLOSE_ENGINE 12
//  ~ Sensor de gás.
#define GAS_SENSOR A5
/* ---------------------------------------------------------------------------------------------------------- */
/*  Variáveis referentes ao sistema de vazamento de gás.                                                      */
//  ~ Indica que o vazamento de gás está ativo.
bool gas_scape;
//  ~ Indica que o duto de gás foi bloqueado.
bool gas_block;
//  ~ Sinal de leitura de gás.
int gas_signal;
//  ~ Tempo de inicio do fechamento da válvula de gás.
unsigned long gas_engine_start_function;
//  ~ Metrónomo do alerta de gás.
unsigned long gas_buzzer_metronome;
/* ---------------------------------------------------------------------------------------------------------- */
/*  Função de entrada do programa.                                                                            */
void setup()
{
  //  ~ Inicializa o Serial.
  Serial.begin(9600);
  while (!Serial);

  //  ~ Portas de entrada de dados ('INPUT') do arduino.


  //  ~ Porta de saída de dados ('OUTPUT') do arduino.
  pinMode(GAS_ALERT, OUTPUT);
  pinMode(GAS_FORCE_CLOSE_ENGINE, OUTPUT);

  //  ~ Inicializa as variáveis globais.
  //  Indica que não há vazamento de gás, caso haja, ele indicará no loop().
  gas_scape = false;
  //  O duto de gás não pode estar bloqueado no começo.
  gas_block = false;
  //  Define o sinal de gás como -1 para evitar interferencia no loop() inicial.
  gas_signal = -1;
  //  Zera os valores de tempo. O valor de 0 indica que a função está ou pode estar desligada.
  gas_engine_start_function = 0;
  gas_buzzer_metronome = 0;  
}
/* ---------------------------------------------------------------------------------------------------------- */
/*  Loop principal do sistema.                                                                                */
void loop()
{
  //  ~ Declara e inicializa as variaveis comparativas de estado.
  int _gas_signal = analogRead(GAS_SENSOR);
  //  ~ Variáveis de estado locais.
  bool gas_engine_power = false;

  //  ~ Verifica alterações no sinal de gás.
  if (gas_signal != _gas_signal)
  {
    //  ~ Equaliza os sinais.
    gas_signal = _gas_signal;
    //  ~ Calcula o percentual de gás.
    int gas_percent = map(_gas_signal, 300, 760, 0, 100);
    //  *** Os valores de 12% e 25% definidos a baixo podem ser ajustados nos #define GAS_SENSOR_LIMIT e GAS_ENGINE_LIMIT
    //  ~ Se o sensor de gás indicar um valor superior ou igual a 12%, ele aciona o alarme de vazamento.
    if (gas_percent >= GAS_SENSOR_LIMIT) gas_scape = true;
    //  ~ Se o sensor de gás indicar valores inferiores a 12%, ele ignora, e considera um ruído.
    else { gas_scape = false; gas_block = false; gas_buzzer_metronome = 0; noTone(GAS_ALERT); }
    //  ~ Se o sensor de gás indicar um valor superior a 25%, ele aciona o motor que realiza o fechamento
    //  de emergência da válvula de gás na entrada da tubulação.
    if ((gas_percent >= GAS_ENGINE_LIMIT) and (!gas_block)) gas_engine_power = true;
  }
  //  ~ Verifica se deve ligar os motores.
  if (gas_engine_power) gas_engine_start_function = millis();
  //  ~ Funcionamento dos motores caso ordenado.
  if (gas_engine_start_function > 0)
  {
    //  ~ Liga o motor.
    digitalWrite(GAS_FORCE_CLOSE_ENGINE, HIGH);
    //  ~ Informa o bloqueio do duto.
    gas_block = true;
    //  ~ Verifica se o motor deve desligar (atingiu sua função).
    if (millis() >= (gas_engine_start_function + GAS_ENGINE_TIME_INTERVAL))
    {
      //  ~ Desliga o motor.
      digitalWrite(GAS_FORCE_CLOSE_ENGINE, LOW);
      //  ~ Zera o valor de 'gas_engine_start_function' indicando que a função está desligada.
      gas_engine_start_function = 0;
    }
  }
  //  ~ Verifica se deve acionar o alarme de vazamento de gás ou não.
  if (gas_scape)
  {
    //  ~ Calcula o tempo para o toque da próxima nota (faz uso de um redutor).
    int next;
    //  ~ Calcula o percentual de gás. (infelizmente faz-se necessário, já que o local em que a outra é calculada não é
    //  chamado em todas as execuções do loop().
    int gas_percent = map(_gas_signal, 300, 760, 0, 100);
    //  ~ Verifica o percentual.
    //  Se for inferior a 25% é tocado uma "semibreve".
    //  No configurado, isso equivale a 1000ms.
    if (gas_percent < 25) next = GAS_ALERT_MAX_TIME; 
    //  Se for superior ou igual a 25% e inferior a 40%, é tocado uma "mínima". (Meia "semibreve")
    //  No configurado, isso equivale a 500ms.
    else if ((gas_percent >= 25) and (gas_percent < 40)) next = (GAS_ALERT_MAX_TIME / 2);
    //  Se for superior ou igual a 40% e inferior a 60%, é tocado uma "semimínima". (Meia "mínima")
    //  No configurado, isso equivale a 250ms.
    else if ((gas_percent >= 40) and (gas_percent < 60)) next = (GAS_ALERT_MAX_TIME / 4);
    //  Se for superior ou igual a 60% e inferior a 90%, é tocado uma "colcheia". (Meia "semimínima")
    //  No configura, isso equivale a 125ms.
    else if ((gas_percent >= 60) and (gas_percent < 90)) next = (GAS_ALERT_MAX_TIME / 8);
    //  Por fim, se for ainda superior, é tocado uma "semicolcheia". (Meia "colcheia")
    //  No configurado, isso equivale a 62,5ms, contudo, devido ao arredondademento, o valor usado é de 62ms.
    else next = (GAS_ALERT_MAX_TIME / 16);
    //  ~ Se o metronomo estiver zerado, inicializa-o.
    if (gas_buzzer_metronome == 0) { gas_buzzer_metronome = millis(); tone(GAS_ALERT, GAS_ALERT_FREQUENCY, next); }
    //  ~ Verifica se deve tocar a próxima nota. O intervalo entre as notas terá a mesma duração da nota.
    if ((millis() >= (gas_buzzer_metronome + next)) and (!((bool) digitalRead(GAS_ALERT))))
    {
      //  ~ Reseta o metronomo.
      gas_buzzer_metronome = millis();
      //  ~ Toca a nota.
      tone(GAS_ALERT, GAS_ALERT_FREQUENCY, next);
    }
    //  ~ Verifica se deve fazer um intervalo.
    if ((millis() >= (gas_buzzer_metronome + next)) and ((bool) digitalRead(GAS_ALERT)))
    {
      //  ~ Reseta o metronomo.
      gas_buzzer_metronome = millis();
      //  ~ Toca a nota.
      noTone(GAS_ALERT);
    }
  }
}
