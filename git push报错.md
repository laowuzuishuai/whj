# 解决git push报错“fatal: unable to access 'https://github.com/xxx/xxx.git/': Recv failure: Connection was reset”

## 1、查看git config --global -l

​	将自己的信息写入，远程仓库地址，用户名等

```
git config --global user.name "user_name"
git config --global user.email "email_id"
git config --global credential.https://github.com/user_name.provider generic
```

注意user_name为GitHub名，email_id为与之关联的邮箱

## 2、写入后提交

```
git status
git add -A .
git commit -m "dslab"
git push
```

