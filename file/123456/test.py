#!/usr/bin/env python
# coding=utf-8
import os
 
 
class File:
    def __init__(self,item_path,item_name):
        self.path=item_path
        self.name=item_name
    def __str__(self):
        n=self.path.count('\\')
        #return "┠{} {}\n".format('━'*n,self.name)
        return '{}<a href={}>{}</a><br>\n'.format(" "*2*n,self.path.replace(' ','%20'),self.name)
 
class fileFolder:
    def __init__(self,item_path,content=None):
        #print('before:',item_path)
        self.path=item_path
        if content is None:
            self.content=[]
        if os.path.isdir(self.path):
            self.name=self.path.split('\\')[-1]
        content=[]
        t = os.path.abspath(__file__).split('/')
        i = 0
        for i in range(len(t)):
            if t[i]=="file":
                break
        insert = ""
        for j in range(i+1,len(t)-1):
            insert = insert + t[j]
        if self.name:
            def key_func(x):
                try:
                    return int(x.split('、')[0])
                except (TypeError,ValueError) as err:
                    return 999
            
            #temp_list=sorted(os.listdir(self.path),key=key_func)
            
            for item in sorted(os.listdir(self.path),key=key_func):
                temp_path=os.path.join(self.path,item)
                #print(temp_path)
                if os.path.isdir(temp_path):
                    #print(temp_path)
                    for k in range(len(tmp)):
                        if k==len(tmp)-1:
                            temp_path = temp_path + insert + "/" + tmp[k]
                        else:
                            temp_path = temp_path + tmp[k] + "/"
                    content.append(fileFolder(temp_path,item))
                else:
                    tmp = temp_path.split('/')
                    if tmp[-1] == "test.py" or tmp[-1]=="index.html":
                        continue
                    temp_path = ""
                    for k in range(len(tmp)):
                        if k==len(tmp)-1:
                            temp_path = temp_path + insert + "/" + tmp[k]
                        else:
                            temp_path = temp_path + tmp[k] + "/"
                    content.append(File(temp_path,item))
            
        self.content=content
    def __str__(self):
        n=self.path.count('\\')
        #output="┏{} 【{}】\n".format('━'*n,self.name)
        output='{}<a href={}>【{}】</a><br>\n'.format(" "*2*n,self.path.replace(' ','%20'),self.name)
        for item in self.content:
            output+="{!s}".format(item)
        return output
    
class fileCount:
    def __init__(self,path='.'):
        self.path=path
        self.HTML_TEMP=('<!DOCTYPE HTML>\n'
               '<html>\n<head>\n<meta charset="utf-8">\n<title>文件目录</title>\n</head>\n'
               '<body>\n{!s}\n</body>\n'
               '</html>')
    def make_html(self):
        with open('index.html','w',encoding='utf-8') as f:
            text=self.HTML_TEMP.format(fileFolder(self.path))
            f.write(text)
 
fileCount().make_html()
 
            
