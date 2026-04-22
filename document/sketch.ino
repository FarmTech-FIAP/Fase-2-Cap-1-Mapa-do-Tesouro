#include <DHT.h>
 
#define PINO_DHT 15
#define PINO_LDR 34
#define PINO_RELE 26
#define BOTAO_N 12
#define BOTAO_P 14
#define BOTAO_K 27
 
DHT dht(PINO_DHT, DHT22);
char clima = 'S';
 
bool n_ligado = false, p_ligado = false, k_ligado = false;
bool anterior_n = HIGH, anterior_p = HIGH, anterior_k = HIGH;
 
void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(PINO_RELE, OUTPUT);
  pinMode(BOTAO_N, INPUT_PULLUP);
  pinMode(BOTAO_P, INPUT_PULLUP);
  pinMode(BOTAO_K, INPUT_PULLUP);
  digitalWrite(PINO_RELE, LOW);
 
  Serial.println("=====================================");
  Serial.println("    SISTEMA DE IRRIGACAO: MILHO      ");
  Serial.println("=====================================");
}
 
void loop() {
  // 1. RECEBE O DADO DO PYTHON (S ou C)
  if (Serial.available() > 0) {
    char letra = Serial.read();
    if (letra == 'S') {
      clima = 'S';
      Serial.println("\n[AVISO]: TEMPO LIMPO - IRRIGAÇÃO NECESSÁRIA");
    }
    else if (letra == 'C') {
      clima = 'C';
      Serial.println("\n[AVISO]: ESTA CHOVENDO - NAO IRRIGAR!");
    }
  }
 
  // 2. LOGICA DOS BOTOES NPK
  bool ler_n = digitalRead(BOTAO_N);
  if (ler_n == LOW && anterior_n == HIGH) { n_ligado = !n_ligado; delay(100); }
  anterior_n = ler_n;
 
  bool ler_p = digitalRead(BOTAO_P);
  if (ler_p == LOW && anterior_p == HIGH) { p_ligado = !p_ligado; delay(100); }
  anterior_p = ler_p;
 
  bool ler_k = digitalRead(BOTAO_K);
  if (ler_k == LOW && anterior_k == HIGH) { k_ligado = !k_ligado; delay(100); }
  anterior_k = ler_k;
 
  // 3. LEITURA DOS SENSORES
  float umidade = dht.readHumidity();
  int luz = analogRead(PINO_LDR);
  float valor_ph = map(luz, 0, 4095, 0, 14);
 
  // 4. REGRAS DO MILHO
  bool ph_certo = (valor_ph >= 5.5 && valor_ph <= 7.5);
  bool solo_seco = (umidade < 55.0);
  bool tem_adubo = (n_ligado && p_ligado && k_ligado);
  bool tempo_sem_chuva = (clima == 'S');
 
  // MOSTRANDO OS DADOS NO MONITOR
  Serial.print("CULTURA: MILHO | ");
  Serial.print("Umidade: "); Serial.print(umidade);
  Serial.print("% | pH: "); Serial.println(valor_ph);
 
  if (solo_seco && ph_certo && tem_adubo && tempo_sem_chuva) {
    digitalWrite(PINO_RELE, HIGH);
    Serial.println(">>> STATUS: BOMBA LIGADA");
  } else {
    digitalWrite(PINO_RELE, LOW);
    Serial.print(">>> STATUS: DESLIGADA | Motivo: ");
   
    if (clima == 'C') Serial.println("ESTA CHOVENDO (ECONOMIZANDO AGUA)");
    else if (!tem_adubo) Serial.println("Falta adubo (NPK)");
    else if (!ph_certo) Serial.println("pH fora do ideal");
    else Serial.println("Solo ja esta molhado");
  }
  Serial.println("-------------------------------------");
  delay(1500);
}