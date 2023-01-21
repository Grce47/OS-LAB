ass_no = input("Enter Assignment number : ")

f = open(f"Assgn1_{ass_no}_08.sh","r")

content = f.read().replace('=', ' = ').replace('|', ' | ').replace(';', ' ; ').replace(',', ' , ').replace('<', ' < ').replace('>', ' > ')
wcnt=len(content.split())
print(f"word count = {wcnt}")
print(content)