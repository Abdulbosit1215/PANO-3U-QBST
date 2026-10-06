# 🚀 GitHub Publishing Guide (Choose 1 Method)

This guide explains how to publish the repository content to GitHub.

Recommended order: **Method A (GitHub Desktop) > Method B (Web upload) > Method C (Command line)**

---

## Method A: GitHub Desktop (Recommended)

1. Install GitHub Desktop: https://desktop.github.com
2. Open GitHub Desktop → `File` → `Add local repository` → choose your local project folder.
3. If prompted that it is not a repository, create one in GitHub Desktop.
4. Sign in to GitHub when prompted.
5. Create a new GitHub repository at https://github.com/new
   - Use the target name
   - Set visibility as needed
   - Do **not** initialize with README/LICENSE/gitignore if these already exist locally
6. Return to GitHub Desktop and publish/push the repository.

---

## Method B: Web Upload (No Installation)

1. Go to https://github.com/new and create a repository.
2. Open the new repository page and click **"uploading an existing file"**.
3. Drag all local contents into the upload area (you may need multiple batches).
4. Add a commit message and click **Commit changes**.

Notes:
- Large repositories often need multi-batch uploads.
- Browser upload limits may apply.

---

## Method C: Git Command Line

1. Install Git: https://git-scm.com/download/win
2. In terminal, run:

```bat
cd /d C:\path\to\your\project
git init
git add .
git commit -m "Initial publish"
git branch -M main
```

3. If needed, configure proxy and Git identity:

```bat
git config --global http.proxy  http://127.0.0.1:7890
git config --global https.proxy http://127.0.0.1:7890
git config --global user.name  "Your Name"
git config --global user.email "your-github-email"
```

4. Create repository at https://github.com/new (without auto-init files).
5. Push:

```bat
git remote add origin https://github.com/<username>/<repo>.git
git push -u origin main
```

If prompted for credentials, use your GitHub username and a Personal Access Token (PAT).

---

## Final Pre-Publish Checklist

- Verify no secrets/tokens are included.
- Verify license/notice files are present.
- Confirm repository visibility (public/private) is correct.
- Confirm large files are within GitHub limits (use Git LFS if needed).

## Suggested Repository Metadata

- Description: project summary with key deliverables
- Topics: cubesat, satellite, open-source-hardware, kicad, step, space
