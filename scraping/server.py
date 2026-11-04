import requests
from bs4 import BeautifulSoup
import xml.etree.ElementTree as ET
import json
import re
import struct
import sys
import time
import os
import datetime
from urllib.parse import urljoin
from requests.adapters import HTTPAdapter
from urllib3.util.retry import Retry

BASE_URL = "https://www.fiba.basketball"

# Sitemap público de jogadores da FIBA, um por país.
# Não requer a API de busca (paga).
SITEMAP_URL_TEMPLATE = (
    BASE_URL + "/en/players-sitemap_index.xml/{code}/players-sitemap.xml"
)

DAT_FILE = "fiba_jogadores.dat"
JSON_FILE = "fiba_jogadores.json"  # só usado na migração (python script.py migrar)

HEADERS = {
    "User-Agent": (
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/140.0 Safari/537.36"
    ),
    "Accept-Language": "en-US,en;q=0.9",
}

COMMIT_EVERY = 100  # confirma no header a cada N jogadores novos


# ============================================================
# CÓDIGOS DE PAÍS (padrão FIBA = padrão IOC, com algumas exceções)
# ============================================================
# Códigos inexistentes são inofensivos: o sitemap volta vazio.
# MSH foi mantido por segurança; MHL (código COI das Ilhas Marshall),
# CUW e AIA foram adicionados.

COUNTRY_CODES = [
    "AFG","ALB","ALG","ASA","AND","ANG","ANT","ARG","ARM","ARU","AUS","AUT",
    "AZE","BAH","BRN","BAN","BAR","BLR","BEL","BIZ","BEN","BER","BHU","BOL",
    "BIH","BOT","BRA","IVB","BRU","BUL","BUR","BDI","CPV","CAM","CMR","CAN",
    "CAY","CAF","CHA","CHI","CHN","TPE","COL","COM","CGO","COD","COK","CRC",
    "CIV","CRO","CUB","CYP","CZE","DEN","DJI","DMA","DOM","ECU","EGY","ESA",
    "GEQ","ERI","EST","SWZ","ETH","FIJ","FIN","FRA","GAB","GAM","GEO","GER",
    "GHA","GIB","GBR","GRE","GRN","GUM","GUA","GUI","GBS","GUY","HAI","HON",
    "HKG","HUN","ISL","IND","INA","IRI","IRQ","IRL","ISR","ITA","JAM","JPN",
    "JOR","KAZ","KEN","KOS","KUW","KGZ","LAO","LAT","LBN","LES","LBR","LBA",
    "LIE","LTU","LUX","MAC","MAD","MAW","MAS","MDV","MLI","MLT","MSH","MHL",
    "MTN","MRI","MEX","FSM","MDA","MON","MGL","MNE","MAR","MOZ","MYA","NAM",
    "NRU","NEP","NED","NZL","NCA","NIG","NGR","PRK","MKD","NOR","OMA","PAK",
    "PLW","PLE","PAN","PNG","PAR","PER","PHI","POL","POR","PUR","QAT","ROU",
    "RUS","RWA","SKN","LCA","VIN","SAM","SMR","STP","KSA","SEN","SRB","SEY",
    "SLE","SGP","SVK","SLO","SOL","SOM","RSA","SSD","KOR","ESP","SRI","SUD",
    "SUR","SWE","SUI","SYR","TJK","TAN","THA","TLS","TOG","TGA","TTO","TUN",
    "TUR","TKM","TCA","TUV","UGA","UKR","UAE","USA","URU","ISV","UZB","VAN",
    "VEN","VIE","YEM","ZAM","ZIM","CUW","AIA",
]


# ============================================================
# FORMATO .DAT
# ============================================================
# Arquivo = HEADER (16 bytes) + N registros de 108 bytes. Little-endian.
#
# HEADER: magic "FIBA" | versão u16 | tamanho do registro u16 | count u32 | reservado u32
# RECORD: id u32 | birth i32 (YYYYMMDD, 0 = desconhecido) | height_cm u16 (0 = desconhecido)
#         | flags u16 | team char[4] | nat char[3][4] | name char[80] (UTF-8, '\0' no fim)
#
# O jogador i está no offset 16 + i * 108.

MAGIC = b"FIBA"
VERSION = 1

HEADER = struct.Struct("<4sHHII")        # 16 bytes
RECORD = struct.Struct("<IiHH4s12s80s")  # 108 bytes
COUNT_OFFSET = 8

assert HEADER.size == 16
assert RECORD.size == 108


def fixed_str(s, size):
    """UTF-8 truncado em fronteira de caractere, deixando 1 byte para o '\\0'."""
    b = (s or "").encode("utf-8")[: size - 1]
    return b.decode("utf-8", "ignore").encode("utf-8")


def parse_birth(text):
    """'Jan 5, 1990' -> 19900105. Retorna 0 se ausente/inválido."""
    if not text:
        return 0
    try:
        d = datetime.datetime.strptime(re.sub(r"\s+", " ", text), "%b %d, %Y")
        return d.year * 10000 + d.month * 100 + d.day
    except ValueError:
        return 0


def pack_player(p):
    nats = [n for n in re.split(r"[/,]", p.get("nacionalidade") or "") if n][:3]
    nat = b"".join(fixed_str(n, 4).ljust(4, b"\0") for n in nats)

    height = p.get("altura_cm") or 0
    if not 0 <= height <= 0xFFFF:
        height = 0

    return RECORD.pack(
        int(p["id"]),
        parse_birth(p.get("data_nascimento")),
        height,
        0,                               # flags (reservado)
        fixed_str(p.get("selecao"), 4),
        nat,                             # o struct completa com '\0' até 12 bytes
        fixed_str(p.get("nome"), 80),
    )


class DatWriter:
    def __init__(self, path=DAT_FILE):
        self.path = path

        if not os.path.exists(path):
            with open(path, "wb") as f:
                f.write(HEADER.pack(MAGIC, VERSION, RECORD.size, 0, 0))

        self.f = open(path, "r+b")

        magic, ver, rsize, count, _ = HEADER.unpack(self.f.read(HEADER.size))
        if magic != MAGIC or rsize != RECORD.size:
            self.f.close()
            raise ValueError("Arquivo .dat incompatível com este layout.")

        # Só vale o que o header confirmou (e que de fato está no arquivo).
        # Registros gravados após o último commit são descartados.
        disponiveis = (os.path.getsize(path) - HEADER.size) // RECORD.size
        count = min(count, disponiveis)
        self.f.truncate(HEADER.size + count * RECORD.size)

        data = self.f.read(count * RECORD.size)
        self.ids = {
            RECORD.unpack_from(data, i * RECORD.size)[0] for i in range(count)
        }

        self.count = count
        self.f.seek(0, os.SEEK_END)

    def append(self, player):
        self.f.write(pack_player(player))
        self.ids.add(int(player["id"]))
        self.count += 1

    def commit(self):
        """Só depois disto o count do header inclui os registros novos."""
        self.f.flush()
        os.fsync(self.f.fileno())
        self.f.seek(COUNT_OFFSET)
        self.f.write(struct.pack("<I", self.count))
        self.f.flush()
        os.fsync(self.f.fileno())
        self.f.seek(0, os.SEEK_END)

    def close(self):
        self.commit()
        self.f.close()


# ============================================================
# REQUEST (com retry/backoff para 429 e 5xx)
# ============================================================

session = requests.Session()
session.headers.update(HEADERS)

_retry = Retry(
    total=5,
    backoff_factor=1,
    status_forcelist=[429, 500, 502, 503, 504],
)
session.mount("https://", HTTPAdapter(max_retries=_retry))
session.mount("http://", HTTPAdapter(max_retries=_retry))


def get_soup(url, params=None):
    response = session.get(url, params=params, timeout=30)
    response.raise_for_status()
    return BeautifulSoup(response.text, "html.parser")


def get_xml(url):
    response = session.get(url, timeout=30)
    response.raise_for_status()
    return ET.fromstring(response.content)


# ============================================================
# NORMALIZAÇÃO DE URL / ID
# ============================================================

def normalize_player_url(url):
    url = urljoin(BASE_URL, url)
    url = url.split("?")[0]
    url = url.split("#")[0]
    return url.rstrip("/")


def get_player_id(url):
    match = re.search(r"/players/(\d+)-", url)
    if match:
        return match.group(1)
    match = re.search(r"/players/(\d+)", url)
    if match:
        return match.group(1)
    return None


# ============================================================
# DESCOBERTA DE JOGADORES VIA SITEMAP
# ============================================================

def players_from_sitemap(country_code):
    """Lê o sitemap público de um país e retorna as URLs de perfil."""

    url = SITEMAP_URL_TEMPLATE.format(code=country_code)

    try:
        root = get_xml(url)
    except requests.RequestException as e:
        print(f"   Erro ao ler sitemap de {country_code}: {e}")
        return set()
    except ET.ParseError as e:
        print(f"   XML inválido para {country_code}: {e}")
        return set()

    players = set()

    for loc in root.iter():
        if loc.tag.endswith("loc") and loc.text:
            link_url = normalize_player_url(loc.text.strip())
            if re.search(r"/players/\d+", link_url):
                players.add(link_url)

    return players


def discover_players():
    """Retorna {id: url} de todos os jogadores encontrados nos sitemaps."""
    discovered = {}

    print("\n" + "=" * 70)
    print("DESCOBRINDO JOGADORES (via sitemap público)")
    print("=" * 70)

    for index, code in enumerate(COUNTRY_CODES, start=1):
        found = players_from_sitemap(code)

        novos = 0
        for url in found:
            pid = get_player_id(url)
            if pid and int(pid) not in discovered:
                discovered[int(pid)] = url
                novos += 1

        print(f"[{index}/{len(COUNTRY_CODES)}] {code}: "
              f"{len(found)} no sitemap, {novos} novos "
              f"(total {len(discovered)})")

        time.sleep(0.3)

    return discovered


# ============================================================
# EXTRAÇÃO DE DADOS DO JOGADOR
# ============================================================

def clean_text(value):
    if not value:
        return None
    value = re.sub(r"\s+", " ", value)
    return value.strip()


def extract_player_data(url):
    soup = get_soup(url)
    text = soup.get_text(" ", strip=True)

    data = {
        "id": get_player_id(url),
        "nome": None,
        "data_nascimento": None,
        "altura_cm": None,
        "selecao": None,
        "nacionalidade": None,
    }

    h1 = soup.find("h1")
    if h1:
        data["nome"] = clean_text(h1.get_text(" ", strip=True))

    match = re.search(
        r"Date of Birth\s+([A-Za-z]{3}\s+\d{1,2},\s+\d{4})", text, re.IGNORECASE
    )
    if match:
        data["data_nascimento"] = match.group(1)

    match = re.search(r"Height\s+(\d+)\s*cm", text, re.IGNORECASE)
    if match:
        data["altura_cm"] = int(match.group(1))

    # (?i:...) aplica ignore-case só ao rótulo; o código do país continua
    # exigindo maiúsculas, evitando capturar "COM" de "Competitions" etc.
    match = re.search(r"(?i:National Team)\s+([A-Z]{3})\b", text)
    if match:
        data["selecao"] = match.group(1)

    match = re.search(
        r"(?i:Nationality)\s+([A-Z]{3}(?:\s*[/,]\s*[A-Z]{3})*)\b", text
    )
    if match:
        data["nacionalidade"] = match.group(1).replace(" ", "")

    return data


# ============================================================
# MIGRAÇÃO JSON -> DAT (opcional, para quem já tem o .json)
# ============================================================

def migrate_json_to_dat():
    if not os.path.exists(JSON_FILE):
        print(f"{JSON_FILE} não encontrado.")
        return

    with open(JSON_FILE, "r", encoding="utf-8") as f:
        players = json.load(f)

    writer = DatWriter()
    importados = 0

    try:
        for p in players:
            if not p.get("id") or not p.get("nome"):
                continue
            if int(p["id"]) in writer.ids:
                continue
            writer.append(p)
            importados += 1
    finally:
        writer.close()

    print(f"Importados: {importados} | Total no .dat: {writer.count}")


# ============================================================
# LOOP PRINCIPAL
# ============================================================

def scrape_fiba():
    writer = DatWriter()
    print(f"\nJogadores já no .dat: {writer.count}")

    discovered = discover_players()
    pending = sorted(
        (pid, url) for pid, url in discovered.items() if pid not in writer.ids
    )

    print("\n" + "=" * 70)
    print(f"DESCOBERTOS: {len(discovered)}  |  A BUSCAR: {len(pending)}")
    print("=" * 70)

    novos = 0
    erros = 0

    try:
        for index, (pid, url) in enumerate(pending, start=1):
            try:
                player = extract_player_data(url)
            except requests.RequestException as e:
                erros += 1
                print(f"[{index}/{len(pending)}] ❌ ERRO HTTP {pid}: {e}")
                continue
            except Exception as e:
                erros += 1
                print(f"[{index}/{len(pending)}] ❌ ERRO {pid}: {e}")
                continue

            if not player.get("nome"):
                print(f"[{index}/{len(pending)}] ⚠️ Perfil {pid} sem nome, ignorado.")
                continue

            writer.append(player)
            novos += 1

            if novos % COMMIT_EVERY == 0:
                writer.commit()

            print(f"[{index}/{len(pending)}] ✅ {player['nome']} (total {writer.count})")

            time.sleep(0.5)

    except KeyboardInterrupt:
        print("\n🛑 SCRAPER INTERROMPIDO")

    finally:
        writer.close()  # confirma o que foi gravado, inclusive em Ctrl+C

    print("\n" + "=" * 70)
    print("FINALIZADO")
    print("=" * 70)
    print(f"Total de jogadores: {writer.count}")
    print(f"Novos nesta execução: {novos}")
    print(f"Erros: {erros}")
    print(f"Arquivo: {DAT_FILE}")


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "migrar":
        migrate_json_to_dat()
    else:
        scrape_fiba()