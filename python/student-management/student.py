# import time
# import os

# 数据列表(全局变量)
student_info = []

# 加载数据
def load_data():
    global student_info
    f = open("student_info.dat")
    content = f.read()
    student_info = eval(content)
    f.close()

# 保存数据
def save_data():
    f = open("student_info.dat","w")
    f.write(str(student_info))
    f.close()


# 登录界面
def login_interface():
    username = input("please input username(ID): ")
    password = input("please input password: ")
    if (username == "admin") and (password == "admin"):
        admin_interface()
    else:
        for temp_info in student_info:
            if username == temp_info['ID'] and password == temp_info['password']:
                student_interface(temp_info)
                return
        else:
            print("username(ID) or password error")


# 学生运行界面
def student_interface(temp_info):
    while True:
        interface_info = '''
        -----------------------------------
        ---------Student Management--------
        ----------Student version----------
                 1.change Password
                 2.  Show grades
                 3.     exit
        -----------------------------------
        -----------------------------------
        '''
        print(interface_info)
        # 管理员界面操作
        switch_value = int(input("please input number that you need to input: "))
        if switch_value == 1:
            password_modify(temp_info)
            return
        elif switch_value == 2:
            show_grades(temp_info)
        elif switch_value == 3:
            flag = input("you are sure to exit student system(yes or no): ")
            if flag == "yes":
                return

def password_modify(temp_info):
    num = student_info.index(temp_info)
    temp_info['password'] = input("input password you want to input: ")
    student_info[num] = temp_info
    save_data()
    print("password is modified")
    input("please input any key to exit!!!")

# 学生成绩显示
def show_grades(temp_info):
    print(temp_info)
    input("please input any key to exit!!!")


# 管理员运行界面
def admin_interface():
    # 管理员界面设计
    while True:
        interface_info = '''
        -----------------------------------
        --------Student Management---------
        -----------Admin version-----------
                  1.insert data
                  2.delete data
                  3.modify data
                  4.ranking
                  5.inquire
                  6.display
                  7.exit system
        -----------------------------------
        -----------------------------------
        '''
        print(interface_info)
        # 管理员界面操作
        switch_value = input("please input number that you need to input: ")
        if switch_value == '1':
            insert()
        elif switch_value == '2':
            delete()
        elif switch_value == '3':
            modify()
        elif switch_value == '4':
            ranking()
        elif switch_value == '5':
            inquire()
        elif switch_value == '6':
            display()
        elif switch_value == '7':
            value = input("you are sure exit admin system(yes or no): ")
            if value == "yes":
                return
        else:
            print("please input 1-6")
            input("please input any key to exit!!!")


# 增加学生数据的函数

def insert():
    global student_info
    print("---------------------------")

    info = dict()
    if len(student_info) != 0:
        new_id = str(int(student_info[len(student_info)-1]['ID']) + 1)
    else:
        new_id = "1"

    info['ID'] = new_id
    info['name'] = input("please input name: ")
    info['password'] = "1"
    info['Chinese'] = int(input("please input Chinese score: "))
    info['Math'] = int(input("please input Math score: "))
    info['English'] = int(input("please input English score:"))
    info['Total_score'] = info['Chinese'] + info['Math'] + info['English']

    student_info.append(info)
    save_data()
    print("insert ok!!!!")
    input("please input any key to exit!!!")

# 删除学生数据的函数
def delete():
    global student_info
    delete_id = input("please input ID that you need to delete：")

    for temp_info in student_info:
        if temp_info['ID'] == delete_id:
            flag = input("you are sure to delete the data(yes or no): ")
            if flag == "yes":
                student_info.remove(temp_info)
                save_data()
                print("delete data!!!!")
                input("please input any key to exit!!!")
                return
            else:
                input("please input any key to exit!!!")
    input("no data in list")
    input("please input any key to exit!!!")


# 修改学生管理系统
def modify():
    global student_info
    modify_id = input("please input ID that you need to modify: ")
    for temp_info in student_info:
        if temp_info['ID'] == modify_id:
            flag = input("you are sure to modify the data(yes or no): ")
            if flag == "yes":
                # 通过查找下标完成对列表数据的修改
                num = student_info.index(temp_info)
                new_info = dict()
                new_info['ID'] = modify_id
                new_info['name'] = input("please input name: ")
                new_info['password'] = input("please input password: ")
                new_info['Chinese'] = int(input("please input Chinese score: "))
                new_info['Math'] = int(input("please input Math score: "))
                new_info['English'] = int(input("please input English score:"))
                new_info['Total_score'] = new_info['Chinese'] + new_info['Math'] + new_info['English']

                student_info[num] = new_info
                save_data()
                print("modify data!!!!")
                input("please input any key to exit!!!")
                return
            else:
                input("please input any key to exit!!!")
    print("no data in list")
    input("please input any key to exit!!!")

# 名次排序
def ranking():
    global student_info
    student_info = sort(student_info, "Chinese")
    student_info = sort(student_info, "Math")
    student_info = sort(student_info, "English")
    student_info = sort(student_info, "Total_score")
    student_info.sort(key=lambda x: x['ID'])
    print(student_info)

    save_data()
    print("ranking is ok!!!!!!")
    input("please input any key to exit!!!")

# 算出名次的函数
def sort(temp_student, flag):
    # x = 0
    # for temp_info in temp_student:
    #     rank = 0
    #     value = temp_info[flag]
    #     print(value)
    #     for temp_info2 in temp_student:
    #         if temp_info2[flag] >= value:
    #             rank += 1
    #     temp_info[flag + "_rank"] = rank
    #     temp_student[x] = temp_info
    #     x += 1
    i = 0
    temp_student.sort(key=lambda x: x[flag])
    for temp_info in temp_student:
        temp_info[flag + "_rank"] = len(temp_student) - i
        temp_student[i] = temp_info
        i += 1
    return temp_student

# 查找需要查询的学生信息
def inquire():
    username = input("please input name you want to input: ")
    for temp_info in student_info:
        if username == temp_info['name']:
            print(temp_info)
            input("please input any key to exit!!!")
            return
    else:
        input("please input any key to exit!!!")

# 显示全部的学生信息
def display():
    for temp_info in student_info:
        print(temp_info)
    input("please input any key to exit!!!")


# main 函数
def main():
    load_data()
    while True:
        login_interface()
        value = input("you are sure exit login system(yes or no): ")
        if value == 'yes':
            break

main()
