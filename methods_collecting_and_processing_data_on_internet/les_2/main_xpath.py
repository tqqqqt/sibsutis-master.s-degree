import requests
import json
import re
from lxml import etree

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
    tree=etree.HTML(full_page.content,etree.HTMLParser())

    vac_cards=tree.xpath('//div[@class="XHH-ReactRoot"]/div/div[@class="magritte-redesign"]/div[@class="HH-MainContent HH-Supernova-MainContent"]/div[@class="main-content main-content_broad-spacing"]/div[@class="magritte-grid-layout___JiVGG_2-2-32"]/div[@class="magritte-grid-layout-content___fzkS0_2-2-32"]/div[@class="magritte-grid-row___3Zugo_2-2-32"]/div[@class="sticky-sidebar-wrapper--Xd38F8jJscAmP1cC"]/div/div/div/main/div[@data-qa="vacancy-serp__results"]/div[not(@*)]')

    for i in range(len(vac_cards)):
        vac_cards[i]=vac_cards[i].xpath('./div/div[@data-qa="vacancy-serp__vacancy"]/div[@class="magritte-icon-dynamic___KJ4yJ_12-3-0 magritte-icon-dynamic_full-width___vgWH5_12-3-0 magritte-icon-dynamic_hover-enabled___WblhN_12-3-0 magritte-icon-dynamic_press-enabled___aGKHB_12-3-0"]/div/div/div/div')
        if isinstance(vac_cards[i],list) and len(vac_cards[i])>1:
            vac_cards[i]=vac_cards[i][0]
        elif isinstance(vac_cards[i],list) and len(vac_cards[i])==0:
            vac_cards.remove(vac_cards[i])

    for i in range(len(vac_cards)):
        name_content.append(vac_cards[i].xpath('./h2[@data-qa="bloko-header-2"]/span/a/div/span/span/text()'))
        link_content.append(vac_cards[i].xpath('./h2[@data-qa="bloko-header-2"]/span/a/@href'))
        price_content.append(''.join(vac_cards[i].xpath('./div[@class="wide-container--ZEcmUvt6oNs8OvdB"]')[1].xpath('./div/span/text()')))

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
