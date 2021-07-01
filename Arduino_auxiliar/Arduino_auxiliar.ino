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
//  ~ Intervalo no qual as luzes da garagem ficam ligadas após o acionamento do sensor de presença.
#define GARAGE_LIGHTS_PIR_INTERVAL 2500
//  ~ Frequencia da nota do alarme.
#define EXTERNAL_ALERT_FREQUENCY 720
//  ~ Duração de toque do alarme.
#define EXTERNAL_ALERT_DURATION 200
//  ~ Duração do intervalo de toque do alarme.
#define EXTERNAL_ALERT_INTERVAL_DURATION 500
//  ~ Erro considerável de medida de temperatura. O valor inserido deve ser em °C, podendo ser um número inteiro
//  ou um número real.
#define TEMPERATURE_MEASUREMENT_ERROR 0.5
/* ---------------------------------------------------------------------------------------------------------- */
/*  Definição de referências de uso para as portas do arduino.                                                */
//  ~ LED de informação do aquecedor.
#define HEATER_LED 3
//  ~ LED de informação do refrigerador.
#define COLDER_LED 4
//  ~ Sensor de temperatura.
#define TEMP_SENSOR A4
//  ~ Alarme da casa
#define EXTERNAL_ALERT 6
//  ~ Sensor de presença externa da casa
#define EXTERNAL_PIR 9
//  ~ Luzes da garagem
#define GARAGE_LIGHT 7
//  ~ Sensor de presença interno da garagem.
#define GARAGE_INTERNAL_PIR 8
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
/*  Variáveis referentes ao monitoramento de presença interno da garagem.                                     */
//  ~ Informa se as luzes estão acesas ou apagadas. Como isso depende das relações envolvidas no loop(), 
//  o sistema inicaliza como apagadas.
bool garage_lights;
//  ~ Instante no qual a luz foi ligada.
unsigned long garage_time_lights_on;
/* ---------------------------------------------------------------------------------------------------------- */
/*  Variáveis referentes ao sistema de alarme.                                                                */
//  ~ Informa se o alerme está ligado ou não.
bool external_alert_actived;
//  ~ Informa se o alarme foi acionado ou não.
bool external_alert;
//  ~ Metrónomo do alarme.
unsigned long external_buzzer_metronome;
/* ---------------------------------------------------------------------------------------------------------- */
/*  Variáveis referentes ao sistema de climatização da casa.                                                  */
//  ~ Determina a temperatura configurada.
float temperature;
//  ~ Sinal da temperatura ambiente.
int environment_temperature_signal;
/* ---------------------------------------------------------------------------------------------------------- */
/*  Função de entrada do programa.                                                                            */
void setup()
{
  //  ~ Inicializa o Serial.
  Serial.begin(9600);
  while (!Serial);

  //  ~ Portas de entrada de dados ('INPUT') do arduino.
  pinMode(GARAGE_INTERNAL_PIR, INPUT);
  pinMode(EXTERNAL_PIR, INPUT);

  //  ~ Porta de saída de dados ('OUTPUT') do arduino.
  pinMode(GAS_ALERT, OUTPUT);
  pinMode(GAS_FORCE_CLOSE_ENGINE, OUTPUT);
  pinMode(GARAGE_LIGHT, OUTPUT);
  pinMode(HEATER_LED, OUTPUT);
  pinMode(COLDER_LED, OUTPUT);

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
  external_buzzer_metronome = 0;
  //  Torna a variável da sala como desligada por enquanto.
  garage_lights = false;  
  //  Coloca o alarme como desativado (o valor em si ira vir do arduino de controle, porém, para fins de evitar erro,
  //  definimos ele aqui também.
  external_alert_actived = false;
  //  Coloca o alerta externo como desligado para evitar problemas.
  external_alert = false;
  //  A temperatura configura é originada do Arduino de controle, porém, fazemos um uso de um valor base aqui fora de escala para evitar problemas.
  temperature = -100;
  //  Coloca o sinal da temperatura ambiente em um valor fora de escala.
  environment_temperature_signal = -1;
  
  //  Zera os valores de tempo. O valor de 0 indica que a função está ou pode estar desligada.
  gas_engine_start_function = 0;
  gas_buzzer_metronome = 0;
  garage_time_lights_on = 0;
  external_buzzer_metronome = 0;  
}
/* ---------------------------------------------------------------------------------------------------------- */
/*  Loop principal do sistema.                                                                                */
void loop()
{
  //  ~ Declara e inicializa as variaveis comparativas de estado.
  int _gas_signal = analogRead(GAS_SENSOR);
  int _environment_temperature_signal = analogRead(TEMP_SENSOR);
  bool _garage_lights = garage_lights;
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

  //  ~ Verifica o sensor PIR interno da garagem.
  if (digitalRead(GARAGE_INTERNAL_PIR)) { garage_time_lights_on = millis(); _garage_lights = true; }
  //  ~ Se a luz estiver acesa, verifica se o contador de tempo já atingiu o limite.
  if (_garage_lights) { if (millis() >= (garage_time_lights_on + GARAGE_LIGHTS_PIR_INTERVAL))
    {
      //  ~ Informa que as luzes devem ser desligadas.
      _garage_lights = false;
      //  ~ Sera o 'garage_time_lights_on'.
      garage_time_lights_on = 0;
    }
  }
  //  ~ Se o contador de tempo da garagem estiver zerado, desliga as luzes.
  if (garage_time_lights_on == 0) _garage_lights = false;
  //  ~ Se for para ligar as luzes, liga, se não, desliga. Mas antes, verifica variação.
  if (_garage_lights != garage_lights)
  {
    //  ~ Realiza o evento.
    digitalWrite(GARAGE_LIGHT, _garage_lights);
    //  ~ Equaliza os valores.
    garage_lights = _garage_lights;
  }

  //  ~ Verifica se o sensor PIR externo foi acionado ou não.
  if (digitalRead(external_alert)) external_alert = true;
  //  ~ Se o alarme deve ser ligado ou não.
  if ((external_alert) and (external_alert_actived))
  {
    //  ~ Se o metronomo estiver zerado, inicializa-o.
    if (external_buzzer_metronome == 0) { external_buzzer_metronome = millis(); tone(EXTERNAL_ALERT, EXTERNAL_ALERT_FREQUENCY, EXTERNAL_ALERT_DURATION); }
    //  ~ Verifica se deve tocar a próxima nota. O intervalo entre as notas terá a mesma duração da nota.
    if ((millis() >= (external_buzzer_metronome + EXTERNAL_ALERT_DURATION)) and (!((bool) digitalRead(EXTERNAL_ALERT))))
    {
      //  ~ Reseta o metronomo.
      external_buzzer_metronome = millis();
      //  ~ Toca a nota.
      tone(EXTERNAL_ALERT, EXTERNAL_ALERT_FREQUENCY, EXTERNAL_ALERT_DURATION);
    }
    //  ~ Verifica se deve fazer um intervalo.
    if ((millis() >= (external_buzzer_metronome + EXTERNAL_ALERT_INTERVAL_DURATION)) and ((bool) digitalRead(EXTERNAL_ALERT)))
    {
      //  ~ Reseta o metronomo.
      external_buzzer_metronome = millis();
      //  ~ Toca a nota.
      noTone(EXTERNAL_ALERT);
    }
  }
  else
  {
    //  ~ Desliga o alerta.
    external_alert = false;
    noTone(EXTERNAL_ALERT);
  }

  //  ~ Verifica se a temperatura registrada na variável e a medida é diferente, se for, executa um processo de alteração.
  if (_environment_temperature_signal != environment_temperature_signal)
  {
    //  ~ Equaliza os valores.
    environment_temperature_signal = _environment_temperature_signal;
    //  ~ Calcula-se a temperatura ambiente.
    float environment_temperature = ((_environment_temperature_signal * (5000.0 / 1024.0)) - 500) / 10.0;
    //  ~ Verifica se a temperatura medida é diferente da temperatura em que o ambiente deve estar. Se for superior, liga
    //  o ar-condicionado em modo de refrigeração, se for inferior, liga no modo de aquecimento.
    //  Como a medição não é perfeita, considera-se a definição de TEMPERATURE_MEASUREMENT_ERROR.
    if (environment_temperature > (temperature - TEMPERATURE_MEASUREMENT_ERROR))
    {
      //  ~ Liga a luz de refrigeração.
      digitalWrite(HEATER_LED, LOW);
      digitalWrite(COLDER_LED, HIGH);
    }
    else if (environment_temperature < (temperature + TEMPERATURE_MEASUREMENT_ERROR))
    {
      //  ~ Liga a luz de aquecimento.
      digitalWrite(HEATER_LED, HIGH);
      digitalWrite(COLDER_LED, LOW);
    }
    else
    {
      //  ~ Desliga o ar-condicionado.
      digitalWrite(HEATER_LED, LOW);
      digitalWrite(COLDER_LED, LOW);
    }
  }
}
