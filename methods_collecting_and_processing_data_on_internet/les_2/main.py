import requests
import json
import re
from bs4 import BeautifulSoup

print("Input vacansy search")
name=input()
if name=="" or ' ' in name:
    exit()

curent_page=0
sesion=requests.Session()

name_content=[]
price_content=[]
link_content=[]
json_result=[]
while True:
    exit_flg=""
    url = "https://novosibirsk.hh.ru/search/vacancy?text="+name+"&page="+str(curent_page)
    full_page = sesion.get(url,headers={'User-Agent':'Chrome/127.0.0.0'})
    soup = BeautifulSoup(full_page.text, 'html.parser')

    try:
        page_content = soup.find(class_="XHH-ReactRoot").find(class_="magritte-redesign").find(class_="HH-MainContent")
        page_content = page_content.find(class_="main-content").find(class_="magritte-grid-layout___JiVGG_2-2-32").find(class_="magritte-grid-layout-content___fzkS0_2-2-32")
        page_content = page_content.find(class_="magritte-grid-row___3Zugo_2-2-32").find(class_="sticky-sidebar-wrapper--Xd38F8jJscAmP1cC").find(class_="magritte-grid-column___rhP24_2-2-32")
        page_content = page_content.find(class_="vacancy-serp-content").find_all(class_="magritte-redesign")
    except AttributeError:
        print("No vacanci")
        break

    for i in range(len(page_content)):
        page_content[i]=page_content[i].find(class_="magritte-card___bhGKz_8-0-7").find(class_="magritte-icon-dynamic___KJ4yJ_12-3-0")
        page_content[i]=page_content[i].find(class_="magritte-flex-container___CVFEY_8-0-7").find(class_="magritte-text-dynamic___71-Al_4-1-2")
        page_content[i]=page_content[i].find(class_="vacancy-card--n77Dj8TY8VIUF0yM").find(class_="vacancy-info--ieHKDTkezpEj0Gsx")
    
    for i in range(len(page_content)):
        name_content.append(page_content[i].find(class_="bloko-header-section-2").find(class_="custom-color-magritte-link--vpnos5EPubw4tVcY"))
        name_content[i]=name_content[i].find(class_="magritte-link___b4rEM_6-0-6")
        link_content.append(str(name_content[i]['href']))
        name_content[i]=name_content[i].find(class_="magritte-icon-dynamic___KJ4yJ_12-3-0")
        name_content[i]=name_content[i].find(class_="magritte-text___pbpft_4-1-2").find(class_="magritte-text___tkzIl_6-0-6")

        price_content.append(page_content[i].find_all(class_="wide-container--ZEcmUvt6oNs8OvdB")[1].find(class_="compensation-labels--vwum2s12fQUurc2J"))
        price_content[i]=price_content[i].find(class_="magritte-text___pbpft_4-1-2")


    for i in range(len(name_content)):
        if price_content[i]:
            print(name_content[i],price_content[i])
            match_big=re.search(r"(\d[\d\s]*)\s*–\s*(\d[\d\s]*)\s*([₽,$,₸])",price_content[i])
            match_smal=re.search(r"(\d[\d\s]*)\s*([₽,$,₸])",price_content[i])
            if match_big:
                min_pr=int(str(match_big.group(1)).strip().replace('\u202f',''))
                max_pr=int(str(match_big.group(2)).strip().replace('\u202f',''))
                new_element=name_content[i],min_pr,max_pr,match_big.group(3),link_content[i]
            elif match_smal:
                min_pr=int(str(match_smal.group(1)).strip().replace('\u202f',''))
                new_element=name_content[i],min_pr,"None",match_smal.group(2),link_content[i]
            else:
                new_element=name_content[i],"None","None","None",link_content[i]
        else:
            print(name_content[i],"None")
            new_element=name_content[i],"None","None","None",link_content[i]
        json_result.append(new_element)

    while exit_flg!="1" and exit_flg!="0":
        print("------------------------------------")
        print(" Input 1 to continue, 0 to exit")
        print("------------------------------------")
        exit_flg=input()

    if exit_flg=="0":
        break
    elif exit_flg=="1":
        curent_page+=1
        name_content.clear()
        price_content.clear()

with open('res.json','w') as file:
    res=json.dumps([{'name':v[0], 'price min':v[1], 'price max':v[2], 'price value':v[3], 'link':v[4], 'site':'hh.ru'} for v in json_result],ensure_ascii=False)
    file.write(res)