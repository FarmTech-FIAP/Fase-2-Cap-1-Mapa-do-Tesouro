#include <DHT.h>

// Definição de Pinos
#define DHTPIN 15
#define DHTTYPE DHT22
#define PINO_LDR 34
#define PINO_RELE 26
#define BOTAO_N 12
#define BOTAO_P 14
#define BOTAO_K 27

DHT dht(DHTPIN, DHTTYPE);

// Variável para previsão do tempo (Vem do Python/Serial)
// 'S' = Sol (Pode irrigar), 'C' = Chuva (Suspender irrigação)
char previsaoTempo = 'S'; 

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  pinMode(PINO_RELE, OUTPUT);
  pinMode(BOTAO_N, INPUT_PULLUP);
  pinMode(BOTAO_P, INPUT_PULLUP);
  pinMode(BOTAO_K, INPUT_PULLUP);
  
  digitalWrite(PINO_RELE, LOW); // Garante bomba desligada no início
  Serial.println("FarmTech Solutions - Sistema Iniciado (Cultura: MILHO)!");
}

void loop() {
  // 1. Leitura da previsão do tempo via Serial (Integração Python)
  if (Serial.available() > 0) {
    char dadoRecebido = Serial.read();
    if (dadoRecebido == 'S' || dadoRecebido == 'C') {
      previsaoTempo = dadoRecebido;
      Serial.print("Previsao do tempo atualizada: ");
      Serial.println(previsaoTempo == 'C' ? "Chuva" : "Sol");
    }
  }

  // 2. Leitura dos Sensores
  float umidade = dht.readHumidity();
  int valorLDR = analogRead(PINO_LDR);
  
  // Mapeia o LDR (0 a 4095) para pH (0 a 14)
  float pH = map(valorLDR, 0, 4095, 0, 14);

  // Leitura do NPK (LOW significa botão pressionado devido ao INPUT_PULLUP)
  bool nivelN = !digitalRead(BOTAO_N);
  bool nivelP = !digitalRead(BOTAO_P);
  bool nivelK = !digitalRead(BOTAO_K);

  // 3. Exibição dos Dados
  Serial.println("--- Dados Atuais ---");
  Serial.print("Umidade do Solo: "); Serial.print(umidade); Serial.println("%");
  Serial.print("Nivel de pH: "); Serial.println(pH);
  Serial.print("NPK (N-P-K): "); 
  Serial.print(nivelN); Serial.print("-"); 
  Serial.print(nivelP); Serial.print("-"); 
  Serial.println(nivelK);

  // 4. Lógica de Irrigação (Cultura: MILHO)
  // Milho tolera pH entre 5.5 e 7.5. Umidade crítica abaixo de 55%.
  bool phAceitavel = (pH >= 5.5 && pH <= 7.5);
  bool precisaAgua = (umidade < 55.0);
  bool vaiChover = (previsaoTempo == 'C');

  if (precisaAgua && phAceitavel && !vaiChover) {
    digitalWrite(PINO_RELE, HIGH);
    Serial.println("STATUS: Bomba LIGADA. Irrigando o milharal.");
  } else {
    digitalWrite(PINO_RELE, LOW);
    Serial.println("STATUS: Bomba DESLIGADA.");
    if (vaiChover) Serial.println("Motivo: Previsao de Chuva.");
    if (!precisaAgua) Serial.println("Motivo: Umidade do solo adequada para o milho.");
    if (!phAceitavel) Serial.println("Motivo: pH fora do ideal (5.5 a 7.5). Verifique os nutrientes NPK.");
  }

  delay(3000); // Aguarda 3 segundos para a próxima leitura
}